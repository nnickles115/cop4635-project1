/**
 * @file http_server.cpp
 * @brief This file contains the definition of the HttpServer class.
 * @details This class is responsible for handling the program lifecycle.
 * It sets up dependency injection, creates the server socket, and listens for incoming 
 * connections, then accepts and dispatches them to worker threads.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "config.hpp"
#include "http_server.hpp"
#include "epoll_manager.hpp"
#include "socket.hpp"
#include "logger.hpp"

#include <arpa/inet.h>

#include <csignal>
#include <cstring>
#include <stdexcept>

// Static instance for signal handling.
HttpServer* HttpServer::instance = nullptr;

/**
 * @brief Constructs a new HttpServer object.
 * @details Initializes the server dependencies and creates the server socket.
 */
HttpServer::HttpServer() : running(true) {
    instance = this;
    setupDependencies();
    setupServerSocket();
}

/**
 * @brief Destroys the HttpServer object along with its dependencies.
 */
HttpServer::~HttpServer() noexcept {
    stop();
}

/**
 * @brief Starts the server and listens for incoming connections.
 * @details This method sets up the server to start listening for incoming connections.
 * It logs the server start, sets up signal handling, and enters a loop to wait for
 * incoming events using the epoll mechanism. When an event is detected on the server
 * socket, it accepts the connection and delegates the handling to the thread pool.
 */
void HttpServer::start() {
    Logger::getInstance().log("Starting server on port: " + std::to_string(Config::getInstance().getPort()), Logger::LogLevel::INFO);

    // Register signal handlers
    registerSignals();

    // Add the server socket to the epoll instance to monitor for incoming connections
    epollManager->addSocket(*socket, EPOLLIN);

    // Main loop to keep the server running
    while(running) {
        // Wait for incoming events on the monitored file descriptors
        auto events = epollManager->waitForEvents(500);

        if(!running) break; // Server is shutting down

        // if(signal_received > 0) break; // Signal received, stop the server

        // Iterate through the list of triggered events
        for(auto& event : events) {
            if(event.data.fd == epollManager->getWakeupFd()) continue;

            // Check if the event is on the server socket (indicating a new connection)
            if(socket && event.data.fd == socket->get()) { // Prevent nullptr dereference
                // Accept the new connection and handle it
                acceptConnections();
            }
        }
    }

    // Clean up resources after the server has stopped (prevents data race)
    factory.reset();
    composer.reset();
    resolver.reset();
    epollManager.reset();
    threadPool.reset();
    instance = nullptr;
}

/**
 * @brief Stops the server and cleans up resources.
 */
void HttpServer::stop() {
    static std::once_flag shutdownFlag;
    std::call_once(shutdownFlag, [&]() {
        Logger::getInstance().log("Server shutting down...", Logger::LogLevel::INFO);
        // Wake up the epoll instance to break out of the event loop
        if(epollManager) epollManager->wakeup();
        
        // Set the running flag to false to break out of the main loop
        running = false;

        // Shutdown the thread pool
        if(threadPool) threadPool->shutdown();
    });
}

/**
 * @brief Registers signal handlers for system signals.
 */
void HttpServer::registerSignals() {
    struct sigaction sa;
    sa.sa_handler = HttpServer::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Disable SA_RESTART to prevent interrupted system calls
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}

/**
 * @brief Signal handler for system signals.
 */
void HttpServer::signalHandler(int signum) {
    if(instance) {
        Logger::getInstance().print("\nReceived SIG" + std::string(sigabbrev_np(signum)));
        instance->stop();
    }
}

/**
 * @brief Initializes and injects the server dependencies.
 */
void HttpServer::setupDependencies() {
    Logger::getInstance().log("Initializing server dependencies...", Logger::LogLevel::DEBUG);

    // Create shared dependencies
    factory = std::make_shared<ResponseBuilderFactory>();
    composer = std::make_shared<ResponseComposer>();
    resolver = std::make_shared<FileResolver>();

    // Register response builders
    factory->registerBuilder(http::method::Method::GET, [this]() {
        return std::make_unique<GetResponseBuilder>(resolver, composer);
    });
    factory->registerBuilder(http::method::Method::POST, [this]() {
        return std::make_unique<PostResponseBuilder>(composer);
    });

    // Create the Epoll manager
    epollManager = std::make_unique<EpollManager>();
    
    // Create the thread pool
    size_t threadCount = Config::getInstance().getThreadCount(); 
    threadPool = std::make_unique<ThreadPool>(threadCount, factory, composer);

    Logger::getInstance().log("Server dependencies initialized.", Logger::LogLevel::INFO);
}

/**
 * @brief Initializes the server socket and binds it to the configured port.
 */
void HttpServer::setupServerSocket() {
    Logger::getInstance().log("Initializing server socket...", Logger::LogLevel::DEBUG);

    // AF_INET = IPv4, SOCK_STREAM = TCP, 0 = IP Protocol
    socket = std::make_unique<Socket>(AF_INET, SOCK_STREAM, 0);

    // optval = 1 enables SO_REUSEADDR, allowing the server to bind to the same port after a restart
    int optval = 1;
    // SOL_SOCKET = Socket level, SO_REUSEADDR = Reuse address
    setsockopt(socket->get(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Set the socket to non-blocking mode (epoll is designed for non-blocking I/O)
    socket->setNonBlocking(true);

    // Set up the server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // IPv4
    addr.sin_addr.s_addr = INADDR_ANY; // Bind to any address
    addr.sin_port = htons(Config::getInstance().getPort()); // Convert to network byte order
    socklen_t addrlen = sizeof(addr);

    // Bind the socket to the address
    socket->bind((struct sockaddr*)&addr, addrlen);

    // Listen for incoming connections with a backlog (max number of pending connections)
    socket->listen(BACKLOG);

    Logger::getInstance().log("Socket successfully bound.", Logger::LogLevel::INFO);
}

/**
 * @brief Accepts incoming connections and delegates them to the thread pool.
 */
void HttpServer::acceptConnections() {
    while(running) {
        // Set up client address struct and accept the connection
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int client_fd = accept(socket->get(), (struct sockaddr*)&client_addr, &client_addrlen);
        
        if(!running) break; // Server is shutting down
        
        // EAGAIN = No pending connections, EWOULDBLOCK = Operation would block
        if(client_fd < 0) {
            // These are non-fatal errors, so breaking out of the loop is acceptable
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            
            // Other errors can be logged and the loop can continue
            Logger::getInstance().log("Failed to accept connection: " + std::string(std::strerror(errno)), Logger::LogLevel::ERROR);
            continue;
        }
        
        // Create a new client socket and set it to non-blocking mode
        Logger::getInstance().log("Accepted connection from: " + std::string(inet_ntoa(client_addr.sin_addr)), Logger::LogLevel::DEBUG);
        auto client_socket = std::make_unique<Socket>(client_fd);
        client_socket->setNonBlocking(true);

        // Delegate the connection to the thread pool
        threadPool->enqueue(std::move(client_socket));
    }
}