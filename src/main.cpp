/**
 * @file main.cpp
 * @brief This file contains the main entry point for the HTTP server.
 * @details It is responsible for passing command line arguments to the Config class, 
 * setting the log level, and starting the server.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "config.hpp"
#include "http_server.hpp"
#include "logger.hpp"

#include <iostream>
#include <memory>

/**
 * @brief Main entry point for the HTTP server.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return `EXIT_SUCCESS` if the server started successfully, `EXIT_FAILURE` otherwise.
 */
int main(int argc, char* argv[]) {
    // Parse command line arguments and set logger level
    Config::getInstance().loadConfig(argc, argv);
    Logger::getInstance().setLogLevel(Config::getInstance().determineLogLevel());

    // Create the server instance and start it
    Logger::getInstance().log("Starting HTTP server...", Logger::LogLevel::INFO);
    try {
        auto server = std::make_unique<HttpServer>();
        server->start();
    }
    catch(const std::exception& e) {
        Logger::getInstance().log("Fatal error: " + std::string(e.what()), Logger::LogLevel::ERROR);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}