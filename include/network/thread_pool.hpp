/**
 * @file thread_pool.hpp
 * @brief This file contains the declaration of the ThreadPool class.
 * @details The ThreadPool class is responsible for managing a pool of worker threads.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "response_builder_factory.hpp"
#include "response_composer.hpp"
#include "socket.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * @brief The ThreadPool class is responsible for managing a pool of worker threads.
 */
class ThreadPool {
public:
    // Constructors //

    ThreadPool(
        size_t numThreads,
        std::shared_ptr<ResponseBuilderFactory> factory,
        std::shared_ptr<ResponseComposer> composer
    );
    ~ThreadPool();

    // Getters //

    bool isActive() const { return !workers.empty(); }

    // Lifecycle //

    void shutdown();
    void enqueue(std::unique_ptr<Socket> client_socket);

private:
    // Dependencies //

    std::shared_ptr<ResponseBuilderFactory> factory;
    std::shared_ptr<ResponseComposer> composer;
    
    // Threads //

    std::vector<std::thread> workers;
    std::condition_variable cv;
    std::atomic<bool> stop;

    // Tasks //

    std::mutex queue_mtx;
    std::queue<std::unique_ptr<Socket>> task_queue;

    // Thread Functions //

    void workerThread();
};

#endif // THREAD_POOL_HPP