/**
 * @file http_status.hpp
 * @brief This file contains constants and utility functions for 
 * working with HTTP status codes.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =HTTP Status Documentation================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status |
// ==========================================================

#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace http::status {
    enum class Code {
        // 1xx Informational
        CONTINUE = 100,
        SWITCHING_PROTOCOLS = 101,
        PROCESSING = 102,
        EARLY_HINTS = 103,

        // 2xx Success
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NON_AUTHORITATIVE_INFORMATION = 203,
        NO_CONTENT = 204,
        RESET_CONTENT = 205,
        PARTIAL_CONTENT = 206,
        MULTI_STATUS = 207,
        ALREADY_REPORTED = 208,
        IM_USED = 226,

        // 3xx Redirection
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        FOUND = 302,
        SEE_OTHER = 303,
        NOT_MODIFIED = 304,
        USE_PROXY = 305,
        SWITCH_PROXY = 306,
        TEMPORARY_REDIRECT = 307,
        PERMANENT_REDIRECT = 308,

        // 4xx Client Error
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        PAYMENT_REQUIRED = 402,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        NOT_ACCEPTABLE = 406,
        PROXY_AUTHENTICATION_REQUIRED = 407,
        REQUEST_TIMEOUT = 408,
        CONFLICT = 409,
        GONE = 410,
        LENGTH_REQUIRED = 411,
        PRECONDITION_FAILED = 412,
        PAYLOAD_TOO_LARGE = 413,
        URI_TOO_LONG = 414,
        UNSUPPORTED_MEDIA_TYPE = 415,
        RANGE_NOT_SATISFIABLE = 416,
        EXPECTATION_FAILED = 417,
        IM_A_TEAPOT = 418,
        MISDIRECTED_REQUEST = 421,
        UNPROCESSABLE_ENTITY = 422,
        LOCKED = 423,
        FAILED_DEPENDENCY = 424,
        TOO_EARLY = 425,
        UPGRADE_REQUIRED = 426,
        PRECONDITION_REQUIRED = 428,
        TOO_MANY_REQUESTS = 429,
        REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
        UNAVAILABLE_FOR_LEGAL_REASONS = 451,

        // 5xx Server Error
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503,
        GATEWAY_TIMEOUT = 504,
        HTTP_VERSION_NOT_SUPPORTED = 505,

        // Error or Unknown
        INVALID = 0
    };
}

namespace std {
    /**
     * @brief The hash specialization for the http::status::Code enum class.
     * @details This allows the enum value to be used as a key in an unordered_map due
     * to it being an enum class.
     * @remark Normal enums leak their values into the surrounding scope, which can cause
     * collisions in the hash map, so this is a safer alternative.
     * @note Hash functions have to be defined in the std namespace.
     */
    template<> struct hash<http::status::Code> {
        size_t operator()(const http::status::Code& c) const noexcept {
            return static_cast<size_t>(c);
        }
    };
}

namespace http::status {
    inline const std::unordered_map<Code, std::string_view> REASON_MAP {
        // 1xx Informational
        {Code::CONTINUE, "Continue"},
        {Code::SWITCHING_PROTOCOLS, "Switching Protocols"},
        {Code::PROCESSING, "Processing"},
        {Code::EARLY_HINTS, "Early Hints"},

        // 2xx Success
        {Code::OK, "OK"},
        {Code::CREATED, "Created"},
        {Code::ACCEPTED, "Accepted"},
        {Code::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
        {Code::NO_CONTENT, "No Content"},
        {Code::RESET_CONTENT, "Reset Content"},
        {Code::PARTIAL_CONTENT, "Partial Content"},
        {Code::MULTI_STATUS, "Multi-Status"},
        {Code::ALREADY_REPORTED, "Already Reported"},
        {Code::IM_USED, "IM Used"},

        // 3xx Redirection
        {Code::MULTIPLE_CHOICES, "Multiple Choices"},
        {Code::MOVED_PERMANENTLY, "Moved Permanently"},
        {Code::FOUND, "Found"},
        {Code::SEE_OTHER, "See Other"},
        {Code::NOT_MODIFIED, "Not Modified"},
        {Code::USE_PROXY, "Use Proxy"},
        {Code::SWITCH_PROXY, "Switch Proxy"},
        {Code::TEMPORARY_REDIRECT, "Temporary Redirect"},
        {Code::PERMANENT_REDIRECT, "Permanent Redirect"},
        
        // 4xx Client Error
        {Code::BAD_REQUEST, "Bad Request"},
        {Code::UNAUTHORIZED, "Unauthorized"},
        {Code::PAYMENT_REQUIRED, "Payment Required"},
        {Code::FORBIDDEN, "Forbidden"},
        {Code::NOT_FOUND, "Not Found"},
        {Code::METHOD_NOT_ALLOWED, "Method Not Allowed"},
        {Code::NOT_ACCEPTABLE, "Not Acceptable"},
        {Code::PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
        {Code::REQUEST_TIMEOUT, "Request Timeout"},
        {Code::CONFLICT, "Conflict"},
        {Code::GONE, "Gone"},
        {Code::LENGTH_REQUIRED, "Length Required"},
        {Code::PRECONDITION_FAILED, "Precondition Failed"},
        {Code::PAYLOAD_TOO_LARGE, "Payload Too Large"},
        {Code::URI_TOO_LONG, "URI Too Long"},
        {Code::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
        {Code::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
        {Code::EXPECTATION_FAILED, "Expectation Failed"},
        {Code::IM_A_TEAPOT, "I'm a teapot"},
        {Code::MISDIRECTED_REQUEST, "Misdirected Request"},
        {Code::UNPROCESSABLE_ENTITY, "Unprocessable Entity"},
        {Code::LOCKED, "Locked"},
        {Code::FAILED_DEPENDENCY, "Failed Dependency"},
        {Code::TOO_EARLY, "Too Early"},
        {Code::UPGRADE_REQUIRED, "Upgrade Required"},
        {Code::PRECONDITION_REQUIRED, "Precondition Required"},
        {Code::TOO_MANY_REQUESTS, "Too Many Requests"},
        {Code::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
        {Code::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},

        // 5xx Server Error
        {Code::INTERNAL_SERVER_ERROR, "Internal Server Error"},
        {Code::NOT_IMPLEMENTED, "Not Implemented"},
        {Code::BAD_GATEWAY, "Bad Gateway"},
        {Code::SERVICE_UNAVAILABLE, "Service Unavailable"},
        {Code::GATEWAY_TIMEOUT, "Gateway Timeout"},
        {Code::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},

        // Error or Unknown
        {Code::INVALID, "Invalid"}
    };

    /**
     * @brief Converts an HTTP status code to a string.
     * @param code The HTTP status code to convert.
     * @return The string representation of the status code.
     */
    inline std::string toString(Code code) noexcept {
        auto it = REASON_MAP.find(code);
        return (it != REASON_MAP.end()) ? std::string(it->second) : "Invalid";
    }

    /**
     * @brief Converts an HTTP status code enum to its corresponding code.
     * @param phrase The HTTP status code enum.
     * @return The corresponding HTTP status code.
     */
    inline std::string getCode(Code code) noexcept {
        return std::to_string(static_cast<int>(code));
    }
}

#endif // HTTP_STATUS_HPP