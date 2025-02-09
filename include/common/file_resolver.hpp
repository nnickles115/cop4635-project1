/**
 * @file file_resolver.hpp
 * @brief This file contains the declaration of the FileResolver class.
 * @details It is responsible for sanitizing URIs and reading files from the file system.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef FILE_RESOLVER_HPP
#define FILE_RESOLVER_HPP

#include "http_status.hpp"

#include <string>
#include <string_view>
#include <variant>

/**
 * @brief The FileResolver class is responsible for resolving file paths and reading files.
 */
class FileResolver {
public:
    std::variant<std::string, http::status::Code> sanitizePath(std::string_view uri) const;
    std::variant<http::status::Code, std::string> readFile(const std::string& path) const;
};

#endif // FILE_RESOLVER_HPP