/**
 * @file epoll_manager.cpp
 * @brief This file contains the definition of the EpollManager class.
 * @details It is responsible for managing epoll events and adding/removing sockets.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "epoll_manager.hpp"
#include "logger.hpp"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

// Constructors //

/**
 * @brief Construct a new EpollManager object.
 * @param maxEvents The maximum number of events to wait for.
 * @throws std::runtime_error If the epoll instance could not be created.
 */
EpollManager::EpollManager(int maxEvents) : max_events(maxEvents) {
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0) {
        throw std::runtime_error("Failed to create epoll instance: " + std::string(std::strerror(errno)));
    }

    wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if(wakeup_fd < 0) {
        throw std::runtime_error("Failed to create eventfd: " + std::string(std::strerror(errno)));
    }

    // Add the wakeup event to the epoll instance
    struct epoll_event event;
    event.data.fd = wakeup_fd;
    event.events = EPOLLIN;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wakeup_fd, &event) < 0) {
        throw std::runtime_error("Failed to add eventfd to epoll: " + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Destroy the EpollManager object.
 */
EpollManager::~EpollManager() {
    close(wakeup_fd);
    close(epoll_fd);
    Logger::getInstance().log("EpollManager destroyed.", Logger::LogLevel::DEBUG);
}

// Functions //

/**
 * @brief Add a socket to the epoll instance.
 * @param socket The socket to add.
 * @param events The events to listen for.
 * @throws std::runtime_error If the socket could not be added.
 */
void EpollManager::addSocket(const Socket& socket, uint32_t events) {
    // Create the epoll event
    struct epoll_event event;
    event.data.fd = socket.get();
    event.events = events;

    // Add the socket to the epoll instance
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket.get(), &event) < 0) {
        throw std::runtime_error("Failed to add socket to epoll: " + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Remove a socket from the epoll instance.
 * @param fd The file descriptor of the socket to remove.
 */
void EpollManager::removeSocket(int fd) {
    // EPOLL_CTL_DEL = Remove the target file descriptor from the interest list of the epoll instance.
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
}

/**
 * @brief Wait for events on the epoll instance.
 * @param timeout_ms The timeout in milliseconds to wait for events.
 * @return A vector of epoll events that occurred.
 * @throws std::runtime_error If epoll_wait() failed.
 * @note If timeout_ms is -1, epoll_wait() will block indefinitely.
 */
std::vector<epoll_event> EpollManager::waitForEvents(int timeout_ms) const {
    std::vector<epoll_event> events(max_events);
    int event_count = epoll_wait(epoll_fd, events.data(), max_events, timeout_ms);
    
    if(event_count < 0 && errno != EINTR) {
        throw std::runtime_error("epoll_wait() failed: " + std::string(std::strerror(errno)));
    }

    if(event_count > 0) {
        events.resize(event_count); // Resize to actual number of events
    }

    for(auto& event : events) {
        if(event.data.fd == wakeup_fd) {
            // Clear the eventfd by reading the value (self-pipe with events)
            uint64_t value;
            read(wakeup_fd, &value, sizeof(value));
        }
    }

    return events;
}

/**
 * @brief Wake up the epoll instance.
 * @note This is used to interrupt epoll_wait() and unblock the thread.
 */
void EpollManager::wakeup() {
    uint64_t one = 1;
    if(write(wakeup_fd, &one, sizeof(one)) < 0) {
        Logger::getInstance().log("Failed to write to wakeup_fd: " + std::string(std::strerror(errno)), Logger::LogLevel::ERROR);
    }
}