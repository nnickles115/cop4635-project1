/**
 * @file n_utils.hpp
 * @brief This file contains forward declarations for various utility functions.
 * I had to change the name of this file and namespace because the boost library
 * has its own "utils.hpp" file.
 * 
 * @author Noah Nickles
 * @date Last Modified: 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef NOAH_UTILS_HPP
#define NOAH_UTILS_HPP

#include <sys/stat.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <optional>
#include <string>
#include <sstream>

namespace n_utils {
    namespace io_style {
        /**
         * @brief Formats a line with a title in the center.
         * @param title The title to center.
         * @param fillChar The character to fill the line with.
         * @param lineWidth The width of the line.
         * @return The formatted line.
         */
        inline std::string seperator(const std::string& title, const char& fillChar, const int& lineWidth) noexcept {
            if(lineWidth < 0) return "";
            if(title.empty()) return std::string(lineWidth, fillChar);
            
            int titleLength = title.length();
            int totalPadding = lineWidth - titleLength - 2;
            int padding = totalPadding / 2;

            std::string seperator;
            seperator.reserve(lineWidth);

            seperator.append(padding, fillChar);
            seperator.append(" ");
            seperator.append(title);
            seperator.append(" ");
            seperator.append(padding, fillChar);
            
            // Add extra fillChar if totalPadding is odd
            if(totalPadding % 2 != 0) {
                seperator.push_back(fillChar);
            }
            
            return seperator;
        }
    }

    namespace io_time {
        /**
         * @brief Gets the current timestamp in the format "YYYY-MM-DD HH:MM:SS".
         * @return The current timestamp as a string.
         */
        inline std::string getCurrentTimestamp() {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            
            return oss.str();
        }

        /**
         * @brief Template function that measures the time it takes to execute a function.
         * @param func The function to execute.
         * @return The duration it took to execute the function.
         * @note The function must be passed in as a lambda with reference capture.
         */
        template<typename Func>
        std::chrono::duration<double> measureTime(Func&& func) {
            auto start = std::chrono::high_resolution_clock::now();
            func(); // Execute passed in function
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> elapsed = end - start;
            return elapsed;
        }
    }

    namespace str_manip {
        /**
         * @brief Trims leading and trailing whitespace from a string.
         * @param str The string to trim.
         * @return The trimmed string.
         */
        inline std::string trim(std::string str) {
            str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
            return str;
        }

        /**
         * @brief Trims every type of whitespace from a string.
         * @param str The string to trim.
         * @return The trimmed string.
         */
        inline std::string trimAll(const std::string& str) noexcept {
            const auto start = str.find_first_not_of(" \t\n\r\v\f");
            const auto end = str.find_last_not_of(" \t\n\r\v\f");
            return(start != std::string::npos)
                ? str.substr(start, end - start + 1)
                : "";
        }

        /**
         * @brief Converts a string to lowercase.
         * @param str The string to convert.
         * @return The lowercase string.
         */
        inline std::string toLower(std::string str) noexcept {
            std::transform(str.begin(), str.end(), str.begin(),
                [](unsigned char c) { return std::tolower(c); }
            );
            return str;
        }

        /**
         * @brief Template function to convert any type to a string.
         * @param input The input to convert.
         * @return The input as a string.
         */
        template <typename T>
        inline std::string toString(const T& input) {
            std::ostringstream oss;
            oss << input;
            return oss.str();
        }

        /*
        struct CaseInsensitiveCompare {
            bool operator()(const std::string& a, const std::string& b) const {
                return std::lexicographical_compare(
                    a.begin(), a.end(), 
                    b.begin(), b.end(),
                    [](unsigned char a, unsigned char b) { 
                        return std::tolower(a) < std::tolower(b); 
                    }
                );
            }
        };
        */
    }
}

#endif // NOAH_UTILS_HPP