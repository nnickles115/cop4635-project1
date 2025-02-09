/**
 * @file epoll_manager.hpp
 * @brief This file contains the declaration of the EpollManager class.
 * @details It is responsible for managing epoll events and adding/removing sockets.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =Linux Documentation================================
// https://man7.org/linux/man-pages/man7/epoll.7.html |
// ====================================================

#ifndef EPOLL_MANAGER_HPP
#define EPOLL_MANAGER_HPP

#include "socket.hpp"

#include <sys/epoll.h>

#include <vector>

/**
 * @brief This class is responsible for managing epoll events and adding/removing sockets.
 */
class EpollManager {
public:
    // Constructors //

    explicit EpollManager(int maxEvents = 10);
    ~EpollManager();

    // Functions //

    void addSocket(const Socket& socket, uint32_t events);
    void removeSocket(int fd);
    std::vector<epoll_event> waitForEvents(int timeout_ms = -1) const;
    void wakeup();
    int getWakeupFd() const { return wakeup_fd; }

private:
    // Variables //

    int epoll_fd;
    int max_events;
    int wakeup_fd;
};

#endif // EPOLL_MANAGER_HPP