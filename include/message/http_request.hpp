/**
 * @file http_request.hpp
 * @brief This file contains the declaration of the HttpRequest class.
 * @details It is responsible for parsing raw HTTP request data 
 * into a structured object.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =HTTP Request Documentation=======================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview       |
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods        |
// https://developer.mozilla.org/en-US/docs/Glossary/Request_header |
// ==================================================================

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "http_message.hpp"
#include "http_method.hpp"
#include "logger.hpp"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

/**
 * @brief Represents an HTTP request.
 * @note Inherits from HttpMessage.
 */
class HttpRequest : public HttpMessage {
public:
    // Constructors //

    HttpRequest() = default;
    HttpRequest(const HttpRequest& other) = default;
    HttpRequest(HttpRequest&& other) = default;
    HttpRequest& operator=(const HttpRequest& other) = default;
    HttpRequest& operator=(HttpRequest&& other) = default;
    ~HttpRequest() = default;

    // Getters //

    std::string_view getMethod() const noexcept { return method; }
    std::string_view getURI() const noexcept { return uri; }

    // Setters //

    HttpRequest& setMethod(http::method::Method method) { 
        this->method = http::method::toString(method); 
        return *this;
    }
    HttpRequest& setURI(std::string_view uri) { 
        this->uri = std::string(uri); 
        return *this;
    }
    
    // Overrides //

    std::string getStatusLine() const noexcept override;
    void display() const override;
    
    // Functions //
    
    bool parse(std::string_view rawData);

private:
    // Variables //
    
    std::string method;
    std::string uri;

    // Functions //
    
    bool parseStartLine(std::string_view line);
    void parseHeaders(std::string_view headersBlock);
    bool parseBody(std::string_view rawData, const size_t& bodyStart);
};

#endif // HTTP_REQUEST_HPP