/**
 * @file socket.cpp
 * @brief This file contains the definition of the Socket class.
 * @details The Socket class serves as a wrapper around the socket file descriptor
 * to adhere to RAII principles.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "logger.hpp"
#include "socket.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <system_error>

// Constructors //

/**
 * @brief Constructs a new Socket object.
 * @param domain The socket domain (AF_INET).
 * @param type The socket type (SOCK_STREAM).
 * @param protocol The socket protocol.
 * @throws std::runtime_error if the socket creation fails.
 * @throws std::runtime_error if the socket options cannot be set.
 */
Socket::Socket(int domain, int type, int protocol) : socket_fd(-1) {
    socket_fd = ::socket(domain, type, protocol);
    if(socket_fd < 0) {
        throw std::runtime_error("Failed to create socket: " + std::string(std::strerror(errno)));
    }

    // Set socket options for reuse
    int optval = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        throw std::runtime_error("Failed to set socket options: " + std::string(std::strerror(errno)));
    }
}

/**
 * @brief Constructs a new Socket object from an existing socket file descriptor.
 * @param socket_fd The existing socket file descriptor.
 * @throws std::runtime_error if the socket file descriptor is invalid.
 */
Socket::Socket(int socket_fd) : socket_fd(socket_fd) {
    if(socket_fd < 0) {
        throw std::runtime_error("Invalid socket file descriptor");
    }
}

/**
 * @brief Destroys the Socket object.
 */
Socket::~Socket() noexcept {
    if(socket_fd >= 0) {
        Logger::getInstance().log("Closing socket.", Logger::LogLevel::DEBUG);
        ::close(socket_fd);
    }
}

/**
 * @brief Move constructor for the Socket object.
 * @param other The other Socket object to move.
 */
Socket::Socket(Socket&& other) noexcept : socket_fd(other.socket_fd) {
    other.socket_fd = -1;
}

/**
 * @brief Move assignment operator for the Socket object.
 * @param other The other Socket object to move.
 */
Socket& Socket::operator=(Socket&& other) noexcept {
    if(this != &other) {
        if(socket_fd >= 0) ::close(socket_fd);
        socket_fd = other.socket_fd;
        other.socket_fd = -1;
    }
    return *this;
}

/**
 * @brief Sets the socket to non-blocking mode.
 * @details This function uses the `fcntl()` system call to set the socket to non-blocking mode.
 * This allows the socket to be used with `poll()` or `epoll()` for asynchronous I/O.
 * @param enable `true` to enable non-blocking mode, `false` to disable.
 * @throws std::runtime_error if the socket flags cannot be set.
 */
void Socket::setNonBlocking(bool enable) {
    // F_GETFL = Get file status flags
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if(flags < 0) {
        throw std::runtime_error("Failed to get socket flags");
    }

    // O_NONBLOCK = Non-blocking mode
    if(enable) flags |= O_NONBLOCK; // Bitwise OR to enable
    else flags &= ~O_NONBLOCK;      // Bitwise AND with negation to disable

    // F_SETFL = Set file status flags
    if(fcntl(socket_fd, F_SETFL, flags) < 0) {
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }
}

/**
 * @brief Binds the socket to a specific address.
 * @param addr The address to bind to.
 * @param addrlen The length of the address.
 * @throws std::system_error if the socket cannot be bound.
 */
void Socket::bind(const struct sockaddr* addr, socklen_t addrlen) {
    if(::bind(socket_fd, addr, addrlen) < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "Failed to bind socket");
    }
}

/**
 * @brief Listens for incoming connections on the socket.
 * @param backlog The maximum number of pending connections.
 * @throws std::system_error if the socket cannot listen for connections.
 */
void Socket::listen(int backlog) {
    if(::listen(socket_fd, backlog) < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "Failed to listen on socket");
    }
}

/**
 * @brief Accepts an incoming connection on the socket.
 * @param addr The address of the client.
 * @param addrlen The length of the address.
 * @return The new Socket object representing the client connection.
 * @throws std::system_error if the connection cannot be accepted.
 */
Socket Socket::accept(struct sockaddr* addr, socklen_t* addrlen) {
    int client_fd = ::accept(socket_fd, addr, addrlen);
    if(client_fd < 0) {
        throw std::system_error(std::error_code(errno, std::system_category()), "Failed to accept connection");
    }
    return Socket(client_fd);
}

/**
 * @brief Receives data from the socket.
 * @param buf The buffer to store the data.
 * @param len The length of the buffer.
 * @param flags The flags to use for the receive operation.
 * @return The number of bytes received.
 * @throws std::system_error if the data cannot be received.
 */
ssize_t Socket::recv(void* buf, size_t len, int flags) const {
    ssize_t bytesRead = ::recv(socket_fd, buf, len, flags);
    if(bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        throw std::system_error(std::error_code(errno, std::system_category()), "Failed to receive data");
    }
    return bytesRead;
}

/**
 * @brief Sends data through the socket. (Dynamic Content)
 * @param buf The buffer containing the data.
 * @param len The length of the buffer.
 * @param flags The flags to use for the send operation.
 * @return The number of bytes sent.
 * @throws std::system_error if the data cannot be sent.
 */
ssize_t Socket::send(const void* buf, size_t len, int flags) const {
    size_t totalSent = 0;
    const char* data = static_cast<const char*>(buf);
    while(totalSent < len) {
        ssize_t bytesSent = ::send(socket_fd, data + totalSent, len - totalSent, flags);
        if(bytesSent < 0) {
            // If socket is non-blocking, handle temporary unavailability
            if(errno == EAGAIN || errno == EWOULDBLOCK) continue; // Retry sending for non-fatal errors
        }
        totalSent += bytesSent;
    }
    return totalSent;
}

/**
 * @brief Sends a file through the socket. (Static Content)
 * @param file_fd The file descriptor of the file to send.
 * @param offset The offset in the file to start sending from.
 * @param count The number of bytes to send.
 * @return The number of bytes sent.
 * @throws std::system_error if the file cannot be sent.
 */
ssize_t Socket::sendfile(int file_fd, off_t* offset, size_t count) const {
    size_t totalSent = 0;
    while(totalSent < count) {
        ssize_t bytesSent = ::sendfile(socket_fd, file_fd, offset, count - totalSent);
        if(bytesSent < 0) {
            // If socket is non-blocking, handle temporary unavailability
            if(errno == EAGAIN || errno == EWOULDBLOCK) continue; // Retry sending for non-fatal errors
        }
        totalSent += bytesSent;
    }
    return totalSent;
}