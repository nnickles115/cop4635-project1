/**
 * @file http_method.hpp
 * @brief This file contains constants and utility functions for 
 * working with HTTP methods.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =HTTP Method Documentation=================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods |
// ===========================================================

#ifndef HTTP_METHOD_HPP
#define HTTP_METHOD_HPP

#include <string>
#include <string_view>
#include <unordered_map>

namespace http::method {
    enum class Method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        TRACE,
        CONNECT,

        // Error or unknown method
        INVALID
    };

    inline const std::unordered_map<Method, std::string_view> METHOD_MAP {
        {Method::GET,     "GET"},
        {Method::POST,    "POST"},
        {Method::PUT,     "PUT"},
        {Method::DELETE,  "DELETE"},
        {Method::HEAD,    "HEAD"},
        {Method::OPTIONS, "OPTIONS"},
        {Method::TRACE,   "TRACE"},
        {Method::CONNECT, "CONNECT"}
    };

    /**
     * @brief Converts a string to an HTTP method.
     * @param str The string to convert.
     * @return The HTTP method enum.
     */
    inline Method fromString(const std::string_view& str) noexcept {
        for(const auto& [method, methodStr] : METHOD_MAP) {
            if(methodStr == str) return method;
        }
        return Method::INVALID;
    }

    /**
     * @brief Converts an HTTP method to a string.
     * @param method The HTTP method to convert.
     * @return The string representation of the method.
     */
    inline const std::string toString(const Method& method) noexcept {
        auto it = METHOD_MAP.find(method);
        return (it != METHOD_MAP.end()) ? std::string(it->second) : "INVALID";
    }

    /**
     * @brief Checks if an HTTP method is valid.
     * @param method The HTTP method to check.
     * @return `true` if valid, `false` otherwise.
     */
    inline bool isValid(const Method& method) noexcept {
        return METHOD_MAP.find(method) != METHOD_MAP.end();
    }
}

#endif // HTTP_METHOD_HPP