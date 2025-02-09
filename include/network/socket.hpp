/**
 * @file socket.hpp
 * @brief This file contains the declaration of the Socket class.
 * @details The Socket class serves as a wrapper around the socket file descriptor
 * to adhere to RAII principles.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =Socket Programming Man Pages============================
// https://man7.org/linux/man-pages/man2/accept.2.html     |
// https://man7.org/linux/man-pages/man2/bind.2.html       |
// https://man7.org/linux/man-pages/man2/close.2.html      |
// https://man7.org/linux/man-pages/man2/listen.2.html     |
// https://man7.org/linux/man-pages/man2/recv.2.html       |
// https://man7.org/linux/man-pages/man2/send.2.html       |
// https://man7.org/linux/man-pages/man2/setsockopt.2.html |
// https://man7.org/linux/man-pages/man2/sendfile.2.html   |
// https://man7.org/linux/man-pages/man2/sigaction.2.html  |
// https://man7.org/linux/man-pages/man2/socket.2.html     |
// =========================================================

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>

/**
 * @brief The Socket class serves as a wrapper around the socket file descriptor 
 * to adhere to RAII principles.
 */
class Socket {
public:
    // Constructors //

    Socket(int domain, int type, int protocol);
    explicit Socket(int socket_fd);
    ~Socket() noexcept;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // Deleted //

    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    // Getters //

    int get() const { return socket_fd; }
    
    // Setters //

    void setNonBlocking(bool enable);
    
    // Socket //

    Socket accept(struct sockaddr* addr, socklen_t* addrlen);
    void bind(const struct sockaddr* addr, socklen_t addrlen);
    void listen(int backlog);
    ssize_t recv(void* buf, size_t len, int flags) const;
    ssize_t send(const void* buf, size_t len, int flags) const;
    ssize_t sendfile(int file_fd, off_t* offset, size_t count) const;

private:
    // Socket //

    int socket_fd;
};

#endif // SOCKET_HPP