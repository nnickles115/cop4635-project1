/**
 * @file response_builder_factory.cpp
 * @brief This file contains the definition of the ResponseBuilderFactory class.
 * @details It is responsible for creating ResponseBuilder objects based on the HTTP method.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "http_method.hpp"
#include "response_builder.hpp"
#include "response_builder_factory.hpp"
#include "logger.hpp"

ResponseBuilderFactory::~ResponseBuilderFactory() noexcept {
    builderRegistry.clear();
    Logger::getInstance().log("ResponseBuilderFactory destroyed.", Logger::LogLevel::DEBUG);
}

/**
 * @brief Registers a delegate builder class for a given HTTP method.
 * @param method The HTTP method to register the delegate for.
 * @param delegate The delegate builder class.
 */
void ResponseBuilderFactory::registerBuilder(http::method::Method method, BuilderType delegate) {
    builderRegistry[method] = std::move(delegate);
}

/**
 * @brief Creates a ResponseBuilder object based on the HTTP method.
 * @returns A unique pointer to a ResponseBuilder or `nullptr` if no builder is found.
 */
std::unique_ptr<ResponseBuilder> ResponseBuilderFactory::createBuilder(const http::method::Method& method) const {
    auto it = builderRegistry.find(method);
    if(it != builderRegistry.end()) {
        return it->second();
    }
    return nullptr;
}