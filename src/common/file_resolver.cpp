/**
 * @file file_resolver.cpp
 * @brief This file contains the definition of the FileResolver class.
 * @details It is responsible for sanitizing URIs and reading files from the file system.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "config.hpp"
#include "file_resolver.hpp"
#include "http_status.hpp"
#include "logger.hpp"

#include <sys/stat.h>
#include <limits.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>

/**
 * @brief Sanitizes a URI and resolves it to a full path.
 * @param uri The URI to sanitize.
 * @returns A variant containing the full path or a status reason.
 */
std::variant<std::string, http::status::Code> FileResolver::sanitizePath(std::string_view uri) const {
    Logger::getInstance().log("Sanitizing path: " + std::string(uri), Logger::LogLevel::DEBUG);

    // Get canonical root folder using realpath
    char rootResolved[PATH_MAX];
    if(realpath(Config::getInstance().getRootFolder().c_str(), rootResolved) == nullptr) {
        Logger::getInstance().log("Invalid root folder: " + Config::getInstance().getRootFolder(), Logger::LogLevel::ERROR);
        return http::status::Code::INTERNAL_SERVER_ERROR;
    }
    std::string root(rootResolved);

    // Build target path string
    std::string targetPath;
    if(uri.empty() || uri == "/") {
        targetPath = root;
        if(!root.empty() && root.back() != '/') {
            targetPath.push_back('/');
        }
        targetPath.append(Config::getInstance().getIndexFile());
    }
    else {
        // Remove starting '/' from uri if present to form relative path
        std::string relative(uri);
        if(!relative.empty() && relative.front() == '/') {
            relative.erase(relative.begin());
        }
        targetPath = root;
        if(!root.empty() && root.back() != '/') {
            targetPath.push_back('/');
        }
        targetPath.append(relative);
    }

    // Canonicalize target path using realpath
    char fullResolved[PATH_MAX];
    if(realpath(targetPath.c_str(), fullResolved) == nullptr) {
        Logger::getInstance().log("Failed to resolve path: " + targetPath, Logger::LogLevel::ERROR);
        return http::status::Code::NOT_FOUND;
    }
    std::string fullPath(fullResolved);
    Logger::getInstance().log("Resolved full path: " + fullPath, Logger::LogLevel::DEBUG);

    // Check that fullPath begins with root folder (prevent traversal)
    if(fullPath.find(root) != 0) {
        Logger::getInstance().log("Directory traversal detected: " + fullPath, Logger::LogLevel::ERROR);
        return http::status::Code::FORBIDDEN;
    }

    // Ensure the file exists and is a regular file
    struct stat pathStat;
    if(stat(fullPath.c_str(), &pathStat) != 0) {
        Logger::getInstance().log("File not found: " + fullPath, Logger::LogLevel::ERROR);
        return http::status::Code::NOT_FOUND;
    }
    if(!S_ISREG(pathStat.st_mode)) {
        Logger::getInstance().log("Invalid file type: " + fullPath, Logger::LogLevel::ERROR);
        return http::status::Code::FORBIDDEN;
    }
    
    return fullPath;
}

/**
 * @brief Reads the contents of a file.
 * @param path The path to the file to read.
 * @returns A variant containing the file contents or a status reason.
 */
std::variant<http::status::Code, std::string> FileResolver::readFile(const std::string& path) const {
    std::ifstream file(path, std::ios::binary | std::ios::ate);  // Open file at the end for size

    if(!file.is_open()) {
        Logger::getInstance().log("Failed to open file: " + path + ", error: " + strerror(errno), Logger::LogLevel::ERROR);
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0) ? http::status::Code::FORBIDDEN : http::status::Code::NOT_FOUND;
    }

    // Get file size and reset file position
    std::streamsize size = file.tellg();
    if(size < 0) {
        Logger::getInstance().log("Failed to get file size: " + path + ", error: " + strerror(errno), Logger::LogLevel::ERROR);
        file.close();
        return http::status::Code::INTERNAL_SERVER_ERROR;
    }
    file.seekg(0, std::ios::beg); // Seek back to beginning

    // Read the file into a string
    std::string content(size, '\0');
    if(!file.read(content.data(), size)) {
        Logger::getInstance().log("Failed to read file: " + path + ", error: " + strerror(errno), Logger::LogLevel::ERROR);
        file.close();
        return http::status::Code::INTERNAL_SERVER_ERROR;
    }

    file.close();
    return content;
}