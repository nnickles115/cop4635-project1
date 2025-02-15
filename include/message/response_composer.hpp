/**
 * @file response_composer.hpp
 * @brief This file contains the declaration of the ResponseComposer class.
 * @details It is responsible for composing HTTP responses into strings
 * to be sent via a socket.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef RESPONSE_COMPOSER_HPP
#define RESPONSE_COMPOSER_HPP

#include "http_response.hpp"
#include "http_status.hpp"

/**
 * @brief The ResponseComposer class is used to build raw HTTP responses from HttpResponse objects.
 */
class ResponseComposer {
public:
    // Functions //
    
    std::string composeResponseString(const HttpResponse& response) const;
    std::string composeErrorMessage(HttpResponse& response, http::status::Code code);
};

#endif // RESPONSE_COMPOSER_HPP