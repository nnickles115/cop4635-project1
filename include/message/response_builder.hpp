/**
 * @file response_builder.hpp
 * @brief This file contains multiple declarations for delegating the 
 * construction of an HTTP response.
 * 
 * @remark I didn't want to make any more files... so I put them all in one (for now).
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =HTTP Documentation=============================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/GET  |
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST |
// ================================================================

#ifndef RESPONSE_BUILDER_HPP
#define RESPONSE_BUILDER_HPP

#include "file_resolver.hpp"
#include "http_response.hpp"
#include "http_request.hpp"
#include "http_status.hpp"
#include "response_composer.hpp"

#include <memory>
#include <optional>
#include <variant>

/**
 * @brief The ResponseResult struct is a wrapper for the result of building an HTTP response.
 */
struct ResponseResult {
    std::variant<HttpResponse, http::status::Code> result;

    bool isError() const { return std::holds_alternative<http::status::Code>(result); }
    bool isSuccess() const { return std::holds_alternative<HttpResponse>(result); }

    http::status::Code getError() const { 
        if(isError()) return std::get<http::status::Code>(result);
        return http::status::Code::INVALID;
    }
    HttpResponse getResponse() const { 
        if(isSuccess()) return std::get<HttpResponse>(result); 
        throw std::runtime_error("ResponseResult does not contain a valid response.");
    }
};

/**
 * @brief The ResponseBuilder class is an interface for building an HTTP response.
 */
class ResponseBuilder {
public:
    // Constructors //

    virtual ~ResponseBuilder() = default;
    
    // Abstract //

    virtual ResponseResult buildResponse(const HttpRequest& request) = 0;

protected:
    // Constants //
    
    static constexpr int MAX_FILE_SIZE = 128 * 1024; // 128KB
};

/**
 * @brief The GetResponseBuilder class is a concrete implementation of the ResponseBuilder interface
 * for handling GET requests.
 * @note Inherits from ResponseBuilder.
 */
class GetResponseBuilder : public ResponseBuilder {
public:
    // Constructors //

    GetResponseBuilder(std::shared_ptr<FileResolver> resolver, std::shared_ptr<ResponseComposer> composer);

    // Overrides //

    ResponseResult buildResponse(const HttpRequest& request) override;

private:
    // Dependencies //

    std::shared_ptr<FileResolver> resolver;
    std::shared_ptr<ResponseComposer> composer;
};

/**
 * @brief The PostResponseBuilder class is a concrete implementation of the ResponseBuilder interface
 * for handling POST requests.
 * @note Inherits from ResponseBuilder.
 */
class PostResponseBuilder : public ResponseBuilder {
public:
    // Constructors //

    explicit PostResponseBuilder(std::shared_ptr<ResponseComposer> composer);

    // Overrides //

    ResponseResult buildResponse(const HttpRequest& request) override;

private:
    // Dependencies //

    std::shared_ptr<ResponseComposer> composer;
};

#endif // RESPONSE_BUILDER_HPP