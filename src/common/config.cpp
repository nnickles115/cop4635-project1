/**
 * @file config.cpp
 * @brief This file contains the definition of the Config class.
 * @details This class is responsible for managing server configuration settings and
 * parsing command line arguments.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#include "config.hpp"
#include "n_utils.hpp"
#include "logger.hpp"

#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

/**
 * @brief Uses a once_flag to ensure the Config object is only created once.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 */
void Config::loadConfig(int argc, char* argv[]) {
    std::call_once(configInitFlag, [&]() {
        Config& instance = getInstance();
        instance.parseCommandLine(argc, argv);
    });
}

/**
 * @brief Determines the log level based on the configuration.
 * @return The determined log level enum.
 */
Logger::LogLevel Config::determineLogLevel() const {
    if(data.debug)      return Logger::LogLevel::DEBUG;
    return Logger::LogLevel::INFO;
}

/**
 * @brief Parses command line arguments using getopt() and creates a Config object.
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return A Config object with the parsed data.
 * @throws std::invalid_argument if a required argument is missing a value.
 */
void Config::parseCommandLine(int argc, char* argv[]) {
    ConfigData parsedData;

    static struct option long_options[] = {
        {"port",          required_argument, 0, 'p'}, // -p value or --port value
        {"debug",         no_argument,       0, 'd'}, // -d or --debug
        {"root",          required_argument, 0, 'r'}, // -r or --root path
        {"index",         required_argument, 0, 'i'}, // -i or --index file
        {"threads",       required_argument, 0, 't'}, // -t count or --threads count
        {0,               0,                 0,  0 }  // Required null terminator
    };

    // Parse command line arguments
    int opt, option_index, verbosityCount = 0;
    while((opt = getopt_long(argc, argv, "p:dr:i:t:", long_options, &option_index)) != -1) {
        switch(opt) {
            case 'p': parsePort(optarg, parsedData);             break;
            case 'd': verbosityCount++; parsedData.debug = true; break;
            case 'r': parseRootFolder(optarg, parsedData);       break;
            case 'i': parseIndexFile(optarg, parsedData);        break;
            case 't': parseThreadCount(optarg, parsedData);      break;
            case '?': handleInvalidOption(optopt, argv);         break;
        }
    }

    // Update the ConfigData struct with the parsed data
    data = parsedData;
}

/**
 * @brief Parses the port number from the command line arguments.
 * @param optarg The argument value.
 * @param data The ConfigData struct to store the parsed data.
 * @throws std::invalid_argument if the port number is invalid
 */
void Config::parsePort(const char* optarg, ConfigData& data) {
    checkInvalidSyntax(optarg);
    try {
        data.port = std::stoi(n_utils::str_manip::trim(optarg));
        if(data.port < 0 || data.port > MAX_PORT) {
            throw std::invalid_argument("Port must be between 1 and 65535 (inclusive).");
        }
    } 
    catch(const std::exception& e) {
        throw std::invalid_argument("Invalid port number.");
    }
}

/**
 * @brief Parses the root folder path from the command line arguments.
 * @param optarg The argument value.
 * @param data The ConfigData struct to store the parsed data.
 */
void Config::parseRootFolder(const char* optarg, ConfigData& data) {
    checkInvalidSyntax(optarg);
    data.rootFolder = n_utils::str_manip::trim(optarg);
    validateRootFolder(data.rootFolder);
}

/**
 * @brief Parses the index file path from the command line arguments.
 * @param optarg The argument value.
 * @param data The ConfigData struct to store the parsed data.
 */
void Config::parseIndexFile(const char* optarg, ConfigData& data) {
    checkInvalidSyntax(optarg);
    data.indexFile = n_utils::str_manip::trim(optarg);
    validateIndexFile(data.rootFolder, data.indexFile);
}

/**
 * @brief Parses the thread count from the command line arguments.
 * @param optarg The argument value.
 * @param data The ConfigData struct to store the parsed data.
 * @throws std::invalid_argument if the thread count is invalid.
 */
void Config::parseThreadCount(const char* optarg, ConfigData& data) {
    checkInvalidSyntax(optarg);
    try {
        data.threadCount = std::stoi(n_utils::str_manip::trim(optarg));
        if(data.threadCount < 0) {
            throw std::invalid_argument("Thread count must be 0 or greater.");
        }
    }
    catch(const std::exception& e) {
        throw std::invalid_argument("Invalid thread count.");
    }
}

/**
 * @brief Handles invalid command line options.
 * @param optopt The invalid option character.
 * @param argv The command line arguments.
 * @throws std::invalid_argument with an error message.
 */
void Config::handleInvalidOption(int optopt, char* argv[]) {
    if(optopt) {
        throw std::invalid_argument("Error: Unknown option -" + std::string(1, static_cast<char>(optopt)));
    } 
    else if(optind > 0) {  // Ensure optind is valid before using it
        throw std::invalid_argument("Error: Invalid long option - " + std::string(argv[optind - 1]));
    }
    throw std::invalid_argument("Invalid command-line argument.");
}

/**
 * @brief Checks for invalid syntax in the command line arguments.
 * @param str The string to check for invalid syntax.
 * @throws std::invalid_argument if the syntax is invalid.
 */
void Config::checkInvalidSyntax(std::string_view str) {
    if(str.find('=') != std::string::npos) {
        throw std::invalid_argument("Error: Invalid syntax. Do not use '=' to assign value. Use space-separated syntax ('--root path' or '-r path').");
    }
}

/**
 * @brief Validates the index file path.
 * @param rootFolder The root folder path.
 * @param fileName The index file name.
 * @throws std::invalid_argument if the file name is empty, does not contain an extension,
 */
void Config::validateIndexFile(const std::string& rootFolder, std::string_view fileName) {
    // Remove the '=' character from the beginning of the arg if specified to clean up the error message
    if(!fileName.empty() && fileName.front() == '=') {
        fileName.remove_prefix(1);
    }

    if(fileName.empty()) {
        throw std::invalid_argument("File name cannot be empty");
    }
    
    // Check if file name contains an extension by looking for a period
    std::string fileNameStr(fileName);
    auto dotPos = fileNameStr.find_last_of('.');
    if(dotPos == std::string::npos || dotPos == 0 || dotPos == fileNameStr.size() - 1) {
        throw std::invalid_argument("File must contain an extension: " + fileNameStr);
    }

    // Build the full path by combining rootFolder and fileName
    std::string fullPath = rootFolder;
    if(!rootFolder.empty() && rootFolder.back() != '/'){
        fullPath.push_back('/');
    }
    fullPath.append(fileNameStr);

    struct stat buffer;
    if(stat(fullPath.c_str(), &buffer) != 0) {
        throw std::invalid_argument("File does not exist: " + fileNameStr);
    }
    if(!S_ISREG(buffer.st_mode)) {
        throw std::invalid_argument("File is not a regular file: " + fileNameStr);
    }
}

/**
 * @brief Validates the root folder path.
 * @param rootFolder The root folder path.
 * @throws std::invalid_argument if the root folder is empty, not a directory, or does not exist.
 */
void Config::validateRootFolder(const std::string& rootFolder) {
    if(rootFolder.empty()) {
        throw std::invalid_argument("Root folder cannot be empty.");
    }

    struct stat buffer;
    if(stat(rootFolder.c_str(), &buffer) != 0) {
        throw std::invalid_argument("Root folder does not exist: " + rootFolder);
    }
    if(!S_ISDIR(buffer.st_mode)) {
        throw std::invalid_argument("Root folder is not a directory: " + rootFolder);
    }
}