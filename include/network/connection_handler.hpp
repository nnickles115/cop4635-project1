/**
 * @file connection_handler.hpp
 * @brief This file contains the declaration of the ConnectionHandler class.
 * @details This class is a delegated handler for a single client connection. It is responsible for
 * processing incoming requests, parsing them, and sending back responses.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =Linux Documentation========================================
// https://man7.org/linux/man-pages/man2/fcntl.2.html         |
// https://man7.org/linux/man-pages/man2/poll.2.html          |
// https://man7.org/linux/man-pages/man2/sendfile.2.html      |
// https://man7.org/linux/man-pages/man2/stat.2.html          |
// https://www.man7.org/linux/man-pages/man0/unistd.h.0p.html |
// ============================================================

#ifndef CONNECTION_HANDLER_HPP
#define CONNECTION_HANDLER_HPP

#include "http_request.hpp"
#include "http_response.hpp"
#include "response_builder_factory.hpp"
#include "response_composer.hpp"
#include "socket.hpp"

#include <memory>

/**
 * @brief The ConnectionHandler class is a delegated handler for a single client connection.
 */
class ConnectionHandler {
public:
    // Constructors //

    ConnectionHandler(
        std::unique_ptr<Socket> client_socket,
        std::shared_ptr<ResponseBuilderFactory> factory,
        std::shared_ptr<ResponseComposer> composer
    );
    ~ConnectionHandler() noexcept;

    // Functions //

    void processRequests();

private:
    // Constants //

    static constexpr int KEEP_ALIVE_TIMEOUT = 60000;    // 60 seconds timeout
    static constexpr int MAX_KEEP_ALIVE_REQUESTS = 100; // Max 100 requests per connection
    static constexpr int BUFFER_SIZE = 128 * 1024;      // 128KB

    // Dependencies //

    std::unique_ptr<Socket> client_socket;
    std::shared_ptr<ResponseBuilderFactory> factory;
    std::shared_ptr<ResponseComposer> composer;

    // Helpers //

    bool waitForData();

    // Functions //

    bool handleRequest();
    HttpRequest parseRequest();
    void sendResponse(HttpResponse& response);
    void sendErrorResponse(const http::status::Code& code);
};

#endif // CONNECTION_HANDLER_HPP