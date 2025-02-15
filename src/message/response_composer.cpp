/**
 * @file response_composer.cpp
 * @brief This file contains the definition of the ResponseComposer class.
 * @details It is responsible for composing HTTP responses into strings
 * to be sent via a socket.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "http_mime.hpp"
#include "http_status.hpp"
#include "http_response.hpp"
#include "logger.hpp"
#include "response_composer.hpp"

#include <sstream>
#include <string>

/**
 * @brief Composes an HTTP response string from the given response object.
 * @param response The HTTP response object.
 * @returns The full HTTP response string.
 */
std::string ResponseComposer::composeResponseString(const HttpResponse& response) const {
    std::ostringstream responseStream;
    responseStream << response.getStatusLine() << "\r\n";

    for(const auto& [key, value] : response.getAllHeaders()) {
        responseStream << key << ": " << value << "\r\n";
    }

    responseStream << "\r\n";

    return responseStream.str();
}


/**
 * @brief Composes an error message response.
 * @param code The HTTP status code.
 * @returns The error message response object.
 */
std::string ResponseComposer::composeErrorMessage(HttpResponse& response, http::status::Code code) {
    // Using the status code as the body
    std::string body = http::status::getCode(code) + " " + http::status::toString(code);

    response.setStatus(code)
            .setHeader("Content-Type", http::mime::toString(http::mime::Media::TEXT_HTML))
            .setHeader("Content-Length", std::to_string(body.length()))
            .setHeader("Connection", "close")
            .setBody(std::move(body));

    std::ostringstream responseStream;
    for(const auto& [key, value] : response.getAllHeaders()) {
        responseStream << key << ": " << value << "\r\n";
    }
    responseStream << "\r\n";
    responseStream << response.getBody();

    return responseStream.str();
}
