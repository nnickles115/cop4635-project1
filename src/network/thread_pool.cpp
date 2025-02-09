/**
 * @file thread_pool.cpp
 * @brief This file contains the definition of the ThreadPool class.
 * @details The ThreadPool class is responsible for managing a pool of worker threads.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "connection_handler.hpp"
#include "http_server.hpp"
#include "logger.hpp"
#include "thread_pool.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

// Constructors //

/**
 * @brief Constructs a new ThreadPool object.
 * @details If `numThreads` is 0, the thread pool will be inactive and tasks will be processed immediately.
 * @param numThreads The number of worker threads to create.
 * @param factory The response builder factory.
 * @param composer The response composer.
 */
ThreadPool::ThreadPool(
    size_t numThreads, 
    std::shared_ptr<ResponseBuilderFactory> factory, 
    std::shared_ptr<ResponseComposer> composer
) : factory(factory), composer(composer), stop(false) {
    assert(this->factory != nullptr);
    assert(this->composer != nullptr);

    if(numThreads == 0) {
        Logger::getInstance().log("Thread pool inactive; running single-threaded.", Logger::LogLevel::WARN);
        return;
    }

    // Create worker threads
    for(size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { workerThread(); });
    }
}

/**
 * @brief Destroys the ThreadPool object.
 */
ThreadPool::~ThreadPool() {
    shutdown();
}

// Lifecycle //

/**
 * @brief Shuts down the thread pool.
 */
void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        if(stop) return; // Prevent double shutdown
        stop = true;
    }
    cv.notify_all();
    
    // Join all worker threads
    for(std::thread& worker : workers) {
        if(worker.joinable()) worker.join();
    }

    // Clear the worker threads vector
    workers.clear();

    Logger::getInstance().log("ThreadPool destroyed.", Logger::LogLevel::DEBUG);
}

/**
 * @brief Enqueues a task to be processed by the thread pool.
 * @param client_socket The client socket to process.
 */
void ThreadPool::enqueue(std::unique_ptr<Socket> client_socket) {
    if(!client_socket) {
        Logger::getInstance().log("Failed to queue task: Client socket is null.", Logger::LogLevel::ERROR);
        return;
    }

    // If the thread pool is inactive, process the request immediately
    if(!isActive()) {
        ConnectionHandler handler(std::move(client_socket), factory, composer);
        handler.processRequests();
        return;
    }

    // Otherwise, queue the task and notify a worker thread
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        if(stop) {
            Logger::getInstance().log("Stopping queues for new tasks.", Logger::LogLevel::DEBUG);
            return; // Prevent new tasks from being enqueued if shutting down
        }
        task_queue.push(std::move(client_socket));

        Logger::getInstance().log("Task enqueued.", Logger::LogLevel::DEBUG);
    }
    cv.notify_one();
}


/**
 * @brief Processes tasks from the task queue.
 */
void ThreadPool::workerThread() {
    while(true) {
        std::unique_ptr<Socket> client_socket;
        {
            // Wait for a task to be enqueued
            std::unique_lock<std::mutex> lock(queue_mtx);
            cv.wait(lock, [this] { return stop || !task_queue.empty(); });

            // Check if the thread should stop
            if(stop && task_queue.empty()) return;

            // Get the next task
            client_socket = std::move(task_queue.front());
            task_queue.pop();
        }

        // Process the task
        if(client_socket) {
            Logger::getInstance().log("Processing task...", Logger::LogLevel::DEBUG);
            ConnectionHandler handler(std::move(client_socket), factory, composer);
            handler.processRequests();
        } 
        else {
            Logger::getInstance().log("Worker thread received null socket.", Logger::LogLevel::ERROR);
        }
    }
}