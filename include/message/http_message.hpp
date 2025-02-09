/**
 * @file http_message.hpp
 * @brief This file contains the declaration of the HttpMessage class.
 * @details It is an abstract class that represents an HTTP message.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "n_utils.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * @brief This class is an abstract class that represents an HTTP message.
 */
class HttpMessage {
public:
    // Constructors //

    HttpMessage() noexcept : version("HTTP/1.1") {};
    HttpMessage(const HttpMessage& other) : version(other.version), headers(other.headers), body(other.body) {
        for(const auto& [key, value] : other.headers) {
            headers[key] = value;
        }
    }
    HttpMessage(HttpMessage&& other) = default;
    HttpMessage& operator=(const HttpMessage& other) {
        if(this != &other) {
            version = other.version;
            headers = other.headers;
            body = other.body;
            for(const auto& [key, value] : other.headers) {
                headers[key] = value;
            }
        }
        return *this;
    }
    HttpMessage& operator=(HttpMessage&& other) = default;
    ~HttpMessage() noexcept = default;

    // Getters //

    std::string getVersion() const noexcept { return version; }
    std::optional<std::string> getHeader(std::string_view key) const {
        std::string lowerKey(key);
        lowerKey = n_utils::str_manip::toLower(lowerKey);
        auto it = headers.find(lowerKey);
        if(it != headers.end()) return it->second;
        return std::nullopt;
    }
    const std::unordered_map<std::string, std::string>& getAllHeaders() const noexcept { return headers; }
    std::string getBody() const noexcept { return body; }

    // Setters //

    void setVersion(std::string_view version) noexcept { this->version = version; }
    HttpMessage& setHeader(std::string_view key, std::string_view value) {
        std::string lowerKey(key);
        lowerKey = n_utils::str_manip::toLower(lowerKey);
        headers[lowerKey] = value;
        return *this;  // Allows chaining
    }
    bool removeHeader(std::string_view key) { 
        std::string lowerKey(key);
        lowerKey = n_utils::str_manip::toLower(lowerKey);
        return headers.erase(lowerKey) > 0;
    }
    void setBody(std::string_view body) { this->body = body; }

    // Interface //

    virtual std::string getStatusLine() const noexcept = 0;
    virtual void display() const = 0;

protected:
    // Variables //

    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

#endif // HTTP_MESSAGE_HPP