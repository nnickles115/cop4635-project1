/**
 * @file connection_handler.cpp
 * @brief This file contains the definition of the ConnectionHandler class.
 * @details This class is a delegated handler for a single client connection. It is responsible for
 * processing incoming requests, parsing them, and sending back responses.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "connection_handler.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_method.hpp"
#include "http_server.hpp"
#include "http_status.hpp"
#include "logger.hpp"
#include "response_builder_factory.hpp"
#include "response_composer.hpp"

#include <fcntl.h>
#include <poll.h> // Using ppoll instead of select (blocking) or epoll since the `ThreadPool` uses epoll
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

// Constructors //

/**
 * @brief Constructs a new ConnectionHandler object.
 * @param client_socket The client socket to handle.
 * @param factory The response builder factory.
 * @param composer The response composer.
 */
ConnectionHandler::ConnectionHandler(
    std::unique_ptr<Socket> client_socket,
    std::shared_ptr<ResponseBuilderFactory> factory,
    std::shared_ptr<ResponseComposer> composer
) : client_socket(std::move(client_socket)), factory(factory), composer(composer) {}

/**
 * @brief Destroys the ConnectionHandler object.
 */
ConnectionHandler::~ConnectionHandler() {
    if(client_socket) {
        Logger::getInstance().log("Closing client connection.", Logger::LogLevel::INFO);
        // Shutdown the socket to prevent further I/O
        shutdown(client_socket->get(), SHUT_RDWR);
        client_socket.reset();
    }
}

// Functions //

/**
 * @brief Processes incoming requests from the client.
 */
void ConnectionHandler::processRequests() {
    int requestCount = 0;
    do {
        if(!waitForData()) {
            if(requestCount > 0) {
                Logger::getInstance().log("Keep-Alive timeout reached.", Logger::LogLevel::INFO);
            }
            break; // No data received within timeout -> close connection
        }

        // Handle the request
        bool keepAlive = handleRequest();
        requestCount++; // Increment request count

        // Check if it reached the max number of requests
        if(requestCount >= MAX_KEEP_ALIVE_REQUESTS) {
            Logger::getInstance().log("Max Keep-Alive requests reached.", Logger::LogLevel::INFO);
            break;
        }

        if(!keepAlive) break; // Connection should be closed (client requested it)
    } while(true);
}

/**
 * @brief Waits for incoming data on the client socket.
 * @return `true` if data is available, `false` if a timeout occurred.
 */
bool ConnectionHandler::waitForData() {
    // Poll in short intervals (e.g. 100ms) until the total elapsed time reaches KEEP_ALIVE_TIMEOUT
    const int shortTimeoutMs = 100;
    int totalElapsedMs = 0;

    while(HttpServer::isRunning() && totalElapsedMs < KEEP_ALIVE_TIMEOUT) {
        struct pollfd pfd;
        pfd.fd = client_socket->get();
        pfd.events = POLLIN;

        // Convert the short timeout to timespec for ppoll
        struct timespec timeout;
        timeout.tv_sec = shortTimeoutMs / 1000;
        timeout.tv_nsec = (shortTimeoutMs % 1000) * 1000000;

        int ret = ppoll(&pfd, 1, &timeout, nullptr);
        if(ret < 0) {
            if(errno == EINTR) {
                Logger::getInstance().log("PPoll interrupted by signal.", Logger::LogLevel::DEBUG);
                return false; // Interrupted by signal
            }
            Logger::getInstance().log("PPoll error: " + std::string(std::strerror(errno)), Logger::LogLevel::DEBUG);
            return false;
        }
        else if(ret > 0 && (pfd.revents & POLLIN)) {
            return true; // Data is available
        }

        totalElapsedMs += shortTimeoutMs;
    }

    if(totalElapsedMs >= KEEP_ALIVE_TIMEOUT) {
        Logger::getInstance().log("Keep-Alive timeout reached.", Logger::LogLevel::INFO);
    }

    return false; // No data available within timeout or shutdown signaled
}

/**
 * @brief Handles an incoming request from the client.
 * @return `true` if the connection should be kept alive, `false` otherwise.
 */
bool ConnectionHandler::handleRequest() {
    try {
        // Parse the incoming request
        HttpRequest request = parseRequest();
        if(Logger::getInstance().getLogLevel() == Logger::LogLevel::DEBUG) request.display();

        // Build the response
        ResponseResult responseResult; // Helper class to wrap std::variant<HttpResponse, http::status::Code>
        http::method::Method method = http::method::fromString(request.getMethod());
        auto builder = factory->createBuilder(method);
        if(builder) {
            responseResult = builder->buildResponse(request);
        }
        else {
            // If the builder is null, the method is not supported
            responseResult = ResponseResult{http::status::Code::NOT_IMPLEMENTED};
        }

        // Compose the response
        HttpResponse response;
        if(responseResult.isSuccess()) {
            response = responseResult.getResponse();
        }
        else {
            response = composer->composeErrorMessage(responseResult.getError());
        }

        // Determine if connection should be kept alive
        bool keepAlive = true;
        if(auto connectionHeader = request.getHeader("Connection"); connectionHeader) {
            if(*connectionHeader == "keep-alive") {
                response.setHeader("Connection", "keep-alive");
            }
            else {
                response.setHeader("Connection", "close");
                keepAlive = false;
            }
        } 
        else {
            response.setHeader("Connection", "keep-alive"); // Default
        }

        // Send the response
        sendResponse(response);
        if(Logger::getInstance().getLogLevel() == Logger::LogLevel::DEBUG) response.display();
        return keepAlive;
    }
    catch(const std::system_error& e) {
        // This happens a lot due to browser behavior
        if(e.code().value() == ECONNRESET) {
            Logger::getInstance().log("Client reset the connection", Logger::LogLevel::DEBUG);
        } 
        else {
            Logger::getInstance().log(std::string(e.what()), Logger::LogLevel::ERROR);
        }
        sendErrorResponse(http::status::Code::BAD_REQUEST);
        return false;
    }
    catch(const std::exception& e) {
        sendErrorResponse(http::status::Code::INTERNAL_SERVER_ERROR);
        return false;
    }
}

/**
 * @brief Parses the incoming HTTP request.
 * @return The parsed HttpRequest object.
 */
HttpRequest ConnectionHandler::parseRequest() {
    char buffer[BUFFER_SIZE] = {0};
    std::string requestData;

    while(true) {
        ssize_t bytesRead = client_socket->recv(buffer, BUFFER_SIZE, 0);
      
        if(bytesRead < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                Logger::getInstance().log("No more data available to read.", Logger::LogLevel::DEBUG);
                break; // No more data available
            }
            throw std::runtime_error("Failed to read request: " + std::string(std::strerror(errno)));
        }
        else if(bytesRead == 0) {
            Logger::getInstance().log("Client closed connection.", Logger::LogLevel::DEBUG);
            throw std::runtime_error("Client closed connection before sending complete request.");
        }
      
        requestData.append(buffer, bytesRead);
        Logger::getInstance().log("Bytes read: " + std::to_string(bytesRead), Logger::LogLevel::DEBUG);

        // Check if headers are complete (look for the empty line)
        if(requestData.find("\r\n\r\n") != std::string::npos) {
            Logger::getInstance().log("Complete header received.", Logger::LogLevel::DEBUG);
            break; // Can start parsing the request
        }
    }

    // Parse the request
    HttpRequest request;
    if(!request.parse(requestData)) {
        throw std::runtime_error("Invalid HTTP request.");
    }
    return request;
}

/**
 * @brief Sends a static or dynamic HTTP response to the client.
 * @param response The HttpResponse object to send.
 */
void ConnectionHandler::sendResponse(HttpResponse& response) {
    // Compose the response headers
    const std::string responseHeadersStr = composer->composeResponseString(response);

    // Send HTTP headers, MSG_NOSIGNAL to prevent SIGPIPE (broken pipe)
    if(client_socket->send(responseHeadersStr.c_str(), responseHeadersStr.size(), MSG_NOSIGNAL) < 0) {
        Logger::getInstance().log("Failed to send response headers.", Logger::LogLevel::ERROR);
        return;
    }

    // Check if the response body is a file path (static content)
    if(response.getIsStatic()) {
        // Get the file path from response body or header
        const std::string filePath = response.getBody().empty() 
                                   ? response.getHeader("File-Path").value_or("")
                                   : response.getBody();

        // Open the file in read-only mode
        int file_fd = open(filePath.c_str(), O_RDONLY);
        if(file_fd < 0) {
            Logger::getInstance().log("Failed to open static file: " + filePath, Logger::LogLevel::ERROR);
            return;
        }

        // Send the file content using sendfile()
        off_t offset = 0;
        size_t totalBytesToSend;
        std::optional<std::string> contentLength = response.getHeader("Content-Length");
        
        if(contentLength.has_value()) {
            totalBytesToSend = std::stoul(contentLength.value());
        }
        else {
            struct stat fileStat;
            if(fstat(file_fd, &fileStat) < 0) {
                Logger::getInstance().log("Failed to get file stats.", Logger::LogLevel::ERROR);
                close(file_fd);
                sendErrorResponse(http::status::Code::INTERNAL_SERVER_ERROR);
                return;
            }
            totalBytesToSend = fileStat.st_size;
        }

        ssize_t bytesSent = client_socket->sendfile(file_fd, &offset, totalBytesToSend);
        if(bytesSent < 0) {
            Logger::getInstance().log("Failed to send static file content.", Logger::LogLevel::ERROR);
            close(file_fd);
            sendErrorResponse(http::status::Code::INTERNAL_SERVER_ERROR);
            return;
        }
        
        // Close the file descriptor
        close(file_fd);
    }
    else {
        // Send dynamic content
        const std::string body = response.getBody();
        if(!body.empty()) {
            if(!response.getHeader("Content-Length").has_value()) {
                response.setHeader("Content-Length", std::to_string(body.length()));
            }
        }
        else {
            response.setHeader("Content-Length", "0");
        }

        ssize_t bytesSent = client_socket->send(body.c_str(), body.length(), MSG_NOSIGNAL);
        if(bytesSent < 0) {
            Logger::getInstance().log("Failed to send dynamic response body.", Logger::LogLevel::ERROR);
            sendErrorResponse(http::status::Code::INTERNAL_SERVER_ERROR);
            return;
        }
    }
}

/**
 * @brief Composes an error response and sends it to the client.
 * @param code The HTTP status code to send.
 */
void ConnectionHandler::sendErrorResponse(const http::status::Code& code) {
    HttpResponse response = composer->composeErrorMessage(code);
    sendResponse(response);
}