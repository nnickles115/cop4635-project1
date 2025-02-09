/**
 * @file http_server.hpp
 * @brief This file contains the declaration of the HttpServer class.
 * @details This class is responsible for handling the program lifecycle.
 * It sets up dependency injection, creates the server socket, and listens for incoming 
 * connections, then accepts and dispatches them to worker threads.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =Linux Documentation================================
// https://man7.org/linux/man-pages/man3/errno.3.html |
// ====================================================

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "epoll_manager.hpp"
#include "file_resolver.hpp"
#include "response_builder_factory.hpp"
#include "response_composer.hpp"
#include "socket.hpp"
#include "thread_pool.hpp"

#include <csignal>
#include <atomic>
#include <memory>

/**
 * @brief The HttpServer class is responsible for handling the program lifecycle.
 */
class HttpServer {
public:
    // Constructors //

    HttpServer();
    ~HttpServer() noexcept;

    // Lifecycle //

    void start();
    void stop();
    static bool isRunning() { return instance->running; }

    // Signals //

    void registerSignals();
    static void signalHandler(int signal);

private:
    // Constants //

    static constexpr int BACKLOG = 10;

    // Signals //

    static HttpServer* instance;

    // Dependencies //

    std::shared_ptr<ResponseBuilderFactory> factory;
    std::shared_ptr<ResponseComposer> composer;
    std::shared_ptr<FileResolver> resolver;

    // Components //

    std::unique_ptr<Socket> socket;
    std::unique_ptr<EpollManager> epollManager;
    std::unique_ptr<ThreadPool> threadPool;
    std::atomic<bool> running;

    // Lifecycle //

    void setupDependencies();
    void setupServerSocket();
    void acceptConnections();
};

#endif // HTTP_SERVER_HPP