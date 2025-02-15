/**
 * @file response_builder.cpp
 * @brief This file contains multiple definitions for delegating the 
 * construction of an HTTP response.
 * 
 * @remark I didn't want to make any more files... so I put them all in one (for now).
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "file_resolver.hpp"
#include "http_encoding.hpp"
#include "http_mime.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_status.hpp"
#include "logger.hpp"
#include "response_builder.hpp"
#include "response_composer.hpp"

#include <sys/stat.h>

#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

using namespace http;

#pragma region GetResponseBuilder
/**
 * @brief Constructs a new GetResponseBuilder.
 * @param resolver The file resolver.
 * @param composer The response composer.
 */
GetResponseBuilder::GetResponseBuilder(
    std::shared_ptr<FileResolver> resolver, 
    std::shared_ptr<ResponseComposer> composer
) : resolver(resolver), composer(composer) {
    assert(this->resolver != nullptr);
    assert(this->composer != nullptr);
}

/**
 * @brief Builds a response to a GET request.
 * @param request The HTTP request.
 * @return The response result.
 */
ResponseResult GetResponseBuilder::buildResponse(const HttpRequest& request) {
    std::string uri = std::string(request.getURI());

    // Sanitize the path
    auto resolvedPath = resolver->sanitizePath(uri);
    if(std::holds_alternative<http::status::Code>(resolvedPath)) {
        return ResponseResult{ std::get<http::status::Code>(resolvedPath) };
    }

    // Get resolved path
    std::string validPath = std::get<std::string>(resolvedPath);

    // Verify the file exists and is a regular file using POSIX stat.
    struct stat st;
    if(stat(validPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
        return ResponseResult{ http::status::Code::NOT_FOUND };
    }

    // Determine MIME type by extracting extension from the path.
    size_t dotPos = validPath.find_last_of('.');
    std::string extension = (dotPos != std::string::npos) ? validPath.substr(dotPos) : "";
    auto mime = http::mime::fromExtension(extension);
    if(mime == http::mime::Media::INVALID) {
        return ResponseResult{ http::status::Code::UNSUPPORTED_MEDIA_TYPE };
    }
    std::string mimeType = http::mime::toString(mime);

    // Check if the file is too large (using stat for file size).
    bool isStatic = false;
    size_t fileSize = static_cast<size_t>(st.st_size);
    if(fileSize > MAX_FILE_SIZE) isStatic = true;

    // Build the response.
    HttpResponse response;
    response.setStatus(http::status::Code::OK)
            .setHeader("Content-Type", mimeType);
            

    if(isStatic) {
        // For static files, delegate reading; no in-memory body.
        response.setHeader("Content-Length", std::to_string(fileSize))
                .setHeader("File-Path", validPath);
        response.setBody("");
        response.setIsStatic(true);
    }
    else {
        auto fileContent = resolver->readFile(validPath);
        if(std::holds_alternative<http::status::Code>(fileContent)) {
            return ResponseResult{ std::get<http::status::Code>(fileContent) };
        }
        std::string contentStr = std::get<std::string>(fileContent);
        response.setHeader("Content-Length", std::to_string(contentStr.size()));
        response.setBody(std::move(contentStr));
        response.setIsStatic(false);
    }

    return ResponseResult{ response };
}
#pragma endregion GetResponseBuilder

#pragma region PostResponseBuilder
/**
 * @brief Constructs a new PostResponseBuilder.
 * @param composer The response composer.
 */
PostResponseBuilder::PostResponseBuilder(std::shared_ptr<ResponseComposer> composer) : composer(composer) {
    assert(this->composer != nullptr);
}

/**
 * @brief Builds a response to a POST request.
 * @param request The HTTP request.
 * @return The response result.
 * @note This function only supports `url-encoded` form data (right now).
 */
ResponseResult PostResponseBuilder::buildResponse(const HttpRequest& request) {
    std::string requestContentType = request.getHeader("Content-Type").value_or("");

    // Extract the base MIME type (removing any parameters like ;charset=UTF-8)
    size_t semicolon = requestContentType.find(';');
    if(semicolon != std::string::npos) {
        requestContentType = requestContentType.substr(0, semicolon);
    }

    if(requestContentType != http::mime::toString(http::mime::Media::APP_FORM)) {
        return ResponseResult{ http::status::Code::UNSUPPORTED_MEDIA_TYPE };
    }
    if(request.getURI() != "/submit") {
        return ResponseResult{ http::status::Code::NOT_FOUND };
    }

    std::unordered_map<std::string, std::string> formData;
    std::istringstream bodyStream(request.getBody());
    std::string keyValue, key, value;

    // Parse the form data
    while(std::getline(bodyStream, keyValue, '&')) {
        size_t pos = keyValue.find('=');
        // Handle key-value pairs with empty keys, and URL-decode both key and value.
        key = (pos == std::string::npos) ? "" : http::encoding::decode(keyValue.substr(0, pos));
        value = (pos == std::string::npos || pos == keyValue.length() - 1) ? "" : http::encoding::decode(keyValue.substr(pos + 1));
        formData[key] = value;
    }

    // Build the response
    std::ostringstream responseBody;
    responseBody << "Received form data:\r\n";
    for(const auto& [key, value] : formData) {
        responseBody << key << ": " << value << "\r\n";
    }
    responseBody << "POST Successful!";

    HttpResponse response;
    response.setStatus(http::status::Code::OK)
            .setHeader("Content-Type", http::mime::toString(http::mime::Media::TEXT_HTML))
            .setHeader("Content-Length", std::to_string(responseBody.str().length()))
            .setHeader("Connection", "close")
            .setBody(responseBody.str());

    return ResponseResult{ response };
}
#pragma endregion PostResponseBuilder