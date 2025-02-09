/**
 * @file logger.cpp
 * @brief This file contains the definition of the Logger class.
 * @details This class is a singleton that provides thread safe 
 * logging functionality.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "logger.hpp"
#include "n_utils.hpp"

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <sstream>

/**
 * @brief This method logs a message to the console.
 * @param message The message to log.
 * @param level The log level of the message.
 */
void Logger::log(std::string_view message, const LogLevel& level) noexcept {
    std::ostream& out = (level == LogLevel::ERROR) ? std::cerr : std::cout;
    log(message, level, out);
}

/**
 * @brief This method logs a message to the specified output stream.
 * @details The `out` param allows the caller to specify what stream type to write to.
 * @param message The message to log.
 * @param level The log level of the message.
 * @param out The output stream to write the log message to.
 */
void Logger::log(std::string_view message, const LogLevel& level, std::ostream& out) noexcept {
    // Prevent messages below the current log level from being printed
    if(level < currentLevel) return;

    // Lock the mutex to prevent multiple threads from writing at the same time
    std::scoped_lock<std::mutex> lock(logMutex);

    std::ostringstream oss;
    // Add the timestamp and log level to the message
    oss << "[" << n_utils::io_time::getCurrentTimestamp() << "]" << "[" << toString(level) << "]";

    // Add padding to the message based on the log level
    if(level == LogLevel::DEBUG || level == LogLevel::ERROR) oss << " " << message;
    else oss << "  " << message;

    // Print the log message to the output stream
    out << oss.str() << std::endl;
    out.flush(); // Ensure immediate output to the console
}

/**
 * @brief This method is a thread-safe way to print a message to the console.
 * @param message The message to print.
 */
void Logger::print(std::string_view message) noexcept {
    std::scoped_lock<std::mutex> lock(logMutex);
    std::cout << message << std::endl;
    std::cout.flush();  // Ensure immediate output to the console
}

/**
 * @brief This method converts a LogLevel enum value to a string.
 * @param level The LogLevel enum value.
 * @returns std::string The string representation of the LogLevel enum value.
 */
std::string Logger::toString(const LogLevel& level) noexcept {
    switch(level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default:              return "UNKNOWN";
    }
}

/**
 * @brief This method converts a string to a LogLevel enum value.
 * @param level The string representation of the LogLevel enum value.
 * @returns LogLevel The LogLevel enum value.
 */
Logger::LogLevel Logger::toEnum(const std::string& level) noexcept {
    if(level == "DEBUG")      return LogLevel::DEBUG;
    else if(level == "INFO")  return LogLevel::INFO;
    else if(level == "WARN")  return LogLevel::WARN;
    else if(level == "ERROR") return LogLevel::ERROR;
    return LogLevel::INFO;
}