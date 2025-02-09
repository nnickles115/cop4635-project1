/**
 * @file http_request.cpp
 * @brief This file contains the definition of the HttpRequest class.
 * @details It is responsible for parsing raw HTTP request data 
 * into a structured object.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "http_request.hpp"
#include "http_method.hpp"
#include "logger.hpp"
#include "n_utils.hpp"

#include <iostream>
#include <sstream>

// Getters //

std::string HttpRequest::getStatusLine() const noexcept {
    std::ostringstream oss;
    oss << getMethod() << " " << getURI() << " " << getVersion();
    return oss.str();
}

// Functions //

/**
 * @brief Parse raw HTTP request data into a structured object.
 * @param rawData The raw HTTP request data.
 * @return `true` if parsing succeeded, `false` if invalid.
 */
bool HttpRequest::parse(std::string_view rawData) {
    size_t startLineEnd = rawData.find("\r\n");
    if(startLineEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed request: Missing start line end.", Logger::LogLevel::ERROR);
        return false;
    }
    std::string_view startLine = rawData.substr(0, startLineEnd);
    if(!parseStartLine(startLine)) {
        Logger::getInstance().log("Malformed request: Invalid start line.", Logger::LogLevel::ERROR);
        return false;
    }

    size_t headersStart = startLineEnd + 2; // Move past "\r\n"
    size_t headersEnd = rawData.find("\r\n\r\n", headersStart);
    if(headersEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed request: Missing headers end.", Logger::LogLevel::ERROR);
        return false;
    }
    std::string_view headersBlock = rawData.substr(headersStart, headersEnd - headersStart);
    parseHeaders(headersBlock);
    
    size_t bodyStart = headersEnd + 4; // Move past "\r\n\r\n"
    if(!parseBody(rawData, bodyStart)) {
        Logger::getInstance().log("Malformed request: Invalid body.", Logger::LogLevel::ERROR);
        return false;
    }

    return true;
}

/**
 * @brief Parse the request line (Method, URI, Version).
 * @param line The request line (GET /index.html HTTP/1.1).
 * @return `true` if valid, `false` if malformed.
 */
bool HttpRequest::parseStartLine(std::string_view line) {
    size_t methodEnd = line.find(" ");
    if(methodEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed start line: Missing space after method.", Logger::LogLevel::ERROR);
        return false;
    }

    size_t uriEnd = line.find(" ", methodEnd + 1);
    if(uriEnd == std::string_view::npos) {
        Logger::getInstance().log("Malformed start line: Missing space after URI.", Logger::LogLevel::ERROR);
        return false;
    }

    std::string_view methodStr = line.substr(0, methodEnd);
    http::method::Method method = http::method::fromString(methodStr); // Convert to enum
    if(!http::method::isValid(method)) {
        Logger::getInstance().log("Invalid HTTP method: " + std::string(methodStr), Logger::LogLevel::ERROR);
        return false;
    }

    setMethod(method);
    setURI(line.substr(methodEnd + 1, uriEnd - methodEnd - 1));
    setVersion(line.substr(uriEnd + 1));

    return true;
}

/**
 * @brief Parse the request headers.
 * @param headersBlock The headers as a string_view.
 */
void HttpRequest::parseHeaders(std::string_view headersBlock) {
    size_t pos = 0;
    while(pos < headersBlock.size()) {
        size_t end = headersBlock.find("\r\n", pos);
        if(end == std::string_view::npos) {
            end = headersBlock.size();
        }
        std::string_view header = headersBlock.substr(pos, end - pos);
        pos = (end == headersBlock.size() ? end : end + 2); // Move past "\r\n" if present

        if(header.empty()) continue;

        size_t separator = header.find(":");
        if(separator != std::string_view::npos) {
            // Allow optional whitespace after the colon
            size_t valueStart = header.find_first_not_of(" ", separator + 1);
            std::string_view value = (valueStart != std::string_view::npos)
                ? header.substr(valueStart)
                : "";
            setHeader(header.substr(0, separator), value);
        }
    }
}

bool HttpRequest::parseBody(std::string_view rawData, const size_t& bodyStart) {
    if(bodyStart < rawData.size()) {
        // Check for Content-Length (impl. Transfer-Encoding later)
        if(auto contentLengthHeader = getHeader("Content-Length")) {
            try {
                size_t contentLength = std::stoul(std::string(*contentLengthHeader));
                if(rawData.size() - bodyStart >= contentLength) {
                    setBody(rawData.substr(bodyStart, contentLength));
                } 
                else {
                    Logger::getInstance().log("Incomplete request body received.", Logger::LogLevel::ERROR);
                    return false;
                }
            } 
            catch(const std::invalid_argument& e) {
                Logger::getInstance().log("Invalid Content-Length header.", Logger::LogLevel::ERROR);
                return false;
            }
            catch(const std::out_of_range& e) {
                Logger::getInstance().log("Content-Length value out of range.", Logger::LogLevel::ERROR);
                return false;
            }
        } 
    }
    else{
        setBody("");
    }

    return true;
}

/**
 * @brief Displays the HTTP request for debugging.
 */
void HttpRequest::display() const {
    std::ostringstream oss;
    const int lineWidth = 24;

    oss << n_utils::io_style::seperator("HTTP REQUEST", '=', lineWidth) << "\n";
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