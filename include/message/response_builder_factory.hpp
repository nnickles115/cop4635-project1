/**
 * @file response_builder_factory.hpp
 * @brief This file contains the declaration of the ResponseBuilderFactory class.
 * @details It is responsible for creating ResponseBuilder objects based on the HTTP method.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef RESPONSE_BUILDER_FACTORY_HPP
#define RESPONSE_BUILDER_FACTORY_HPP

#include "http_method.hpp"
#include "response_builder.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

using BuilderType = std::function<std::unique_ptr<ResponseBuilder>()>;

/**
 * @brief This class is responsible for creating ResponseBuilder objects based on the HTTP method.
 */
class ResponseBuilderFactory {
public:
    // Constructors //

    ResponseBuilderFactory() = default;
    ~ResponseBuilderFactory() noexcept;

    // Functions //

    void registerBuilder(http::method::Method method, BuilderType delegate);
    std::unique_ptr<ResponseBuilder> createBuilder(const http::method::Method& method) const;


private:
    // Variables //

    std::unordered_map<http::method::Method, BuilderType> builderRegistry;
};

#endif // RESPONSE_BUILDER_FACTORY_HPP