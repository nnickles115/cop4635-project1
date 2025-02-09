/**
 * @file config.hpp
 * @brief This file contains the declaration of the Config class.
 * @details This class is responsible for managing server configuration settings and
 * parsing command line arguments.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =Argument Parsing Documentation==========================
// https://www.man7.org/linux/man-pages/man3/getopt.3.html |
// =========================================================

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "logger.hpp"

#include <string>

/**
 * @brief The ConfigData struct contains the configuration settings for the server.
 */
struct ConfigData {
    int port = 60001;
    bool debug = false;
    std::string rootFolder = "./www";
    std::string indexFile = "index.html";
    int threadCount = 4;
};

/**
 * @brief The Config class is responsible for managing server configuration settings and
 * parsing command line arguments.
 */
class Config {
public:
    // Singleton //

    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // Deleted //

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // Getters //

    int getPort() const noexcept { return data.port; }
    bool isDebug() const noexcept { return data.debug; }
    std::string getRootFolder() const { return data.rootFolder; }
    std::string getIndexFile() const { return data.indexFile; }
    size_t getThreadCount() const noexcept { return data.threadCount; }
    Logger::LogLevel determineLogLevel() const;
    
    // Functions //

    void loadConfig(int argc, char* argv[]);

private:
    // Constants //

    static constexpr int MAX_PORT = 65535;

    // Singleton //

    Config() = default; // Private constructor to prevent instantiation

    // Data //

    ConfigData data;
    std::once_flag configInitFlag; // For lazy initialization

    // Parsing //

    void parseCommandLine(int argc, char* argv[]);
    void parsePort(const char* optarg, ConfigData& data);
    void parseRootFolder(const char* optarg, ConfigData& data);
    void parseIndexFile(const char* optarg, ConfigData& data);
    void parseThreadCount(const char* optarg, ConfigData& data);
    void handleInvalidOption(int optopt, char* argv[]);

    // Helpers //

    void checkInvalidSyntax(std::string_view str);
    void validateIndexFile(const std::string& rootFolder, std::string_view fileName);
    void validateRootFolder(const std::string& rootFolder);
};

#endif // CONFIG_HPP