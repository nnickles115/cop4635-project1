/**
 * @file http_response.cpp
 * @brief This file contains the definition of the HttpResponse class.
 * @details It is responsible for holding the HTTP response data.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "http_response.hpp"
#include "http_status.hpp"
#include "logger.hpp"
#include "n_utils.hpp"

#include <iostream>
#include <sstream>

// Constructors //

HttpResponse::HttpResponse() noexcept : status(http::status::Code::OK), isStatic(false) {}

// Overrides //

/**
 * @brief Generates the HTTP status line (HTTP/1.1 200 OK).
 */
std::string HttpResponse::getStatusLine() const noexcept {
    std::ostringstream oss;
    oss << getVersion() << " " << static_cast<int>(status) << " " << http::status::toString(status);
    return oss.str();
}

/**
 * @brief Displays the HTTP response for debugging.
 */
void HttpResponse::display() const {
    std::ostringstream oss;
    const int lineWidth = 24;

    oss << n_utils::io_style::seperator("HTTP RESPONSE", '=', lineWidth) << "\n";
    oss << getStatusLine() << "\n";
    oss << n_utils::io_style::seperator("Headers", '-', lineWidth) << "\n";

    for(const auto& [key, value] : getAllHeaders()) {
        oss << key << ": " << value << "\n";
    }

    oss << n_utils::io_style::seperator("Body", '-', lineWidth) << "\n";
    oss << getBody() << "\n";
    oss << n_utils::io_style::seperator("", '=', lineWidth) << "\n";

    Logger::getInstance().print(oss.str());
}