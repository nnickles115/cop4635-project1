/**
 * @file http_response.hpp
 * @brief This file contains the declaration of the HttpResponse class.
 * @details It is responsible for holding the HTTP response data.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "http_message.hpp"
#include "http_status.hpp"

#include <string>

/**
 * @brief Represents an HTTP response.
 * @note Inherits from HttpMessage.
 */
class HttpResponse : public HttpMessage {
public:
    // Constructors //

    HttpResponse() noexcept;
    HttpResponse(const HttpResponse& other) = default;
    HttpResponse(HttpResponse&&) = default;
    HttpResponse& operator=(const HttpResponse& other) = default;
    HttpResponse& operator=(HttpResponse&&) = default;
    ~HttpResponse() = default;

    // Getters //

    http::status::Code getStatus() const noexcept { return status; }
    bool getIsStatic() const noexcept { return isStatic; }
    
    // Setters //

    HttpResponse& setStatus(http::status::Code status) noexcept {
        this->status = status;
        return *this;
    }
    HttpResponse& setIsStatic(bool isStatic) noexcept { 
        this->isStatic = isStatic; 
        return *this;
    }

    // Overrides //

    std::string getStatusLine() const noexcept override;
    void display() const override;

private:
    // Variables //
    
    http::status::Code status;
    bool isStatic;
};

#endif // HTTP_RESPONSE_HPP