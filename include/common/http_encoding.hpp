/**
 * @file http_encoding.hpp
 * @brief This file contains constants and functions for encoding and decoding 
 * percent-encoded strings. 
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

// =HTTP POST Documentation========================================
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST |
// ================================================================

#ifndef HTTP_ENCODING_HPP
#define HTTP_ENCODING_HPP

#include <string>
#include <unordered_map>

namespace http::encoding {
    inline const std::unordered_map<char, std::string> ENCODING_MAP = {
        {' ',  "%20"},
        {'!',  "%21"},
        {'"',  "%22"},
        {'#',  "%23"},
        {'$',  "%24"},
        {'%',  "%25"},
        {'&',  "%26"},
        {'\'', "%27"},
        {'(',  "%28"},
        {')',  "%29"},
        {'*',  "%2A"},
        {'+',  "%2B"},
        {',',  "%2C"},
        {'-',  "%2D"},
        {'.',  "%2E"},
        {'/',  "%2F"},
        {':',  "%3A"},
        {';',  "%3B"},
        {'<',  "%3C"},
        {'=',  "%3D"},
        {'>',  "%3E"},
        {'?',  "%3F"},
        {'@',  "%40"},
        {'[',  "%5B"},
        {'\\', "%5C"},
        {']',  "%5D"},
        {'^',  "%5E"},
        {'_',  "%5F"},
        {'`',  "%60"},
        {'{',  "%7B"},
        {'|',  "%7C"},
        {'}',  "%7D"},
        {'~',  "%7E"}
    };

    /**
     * @brief Encodes a string into a percent-encoded string.
     * @param str The string to encode.
     * @return The percent-encoded string.
     */
    inline std::string encode(const std::string& str) noexcept {
        std::string encoded;
        for(char c : str) {
            auto it = ENCODING_MAP.find(c);
            if(it != ENCODING_MAP.end()) {
                encoded.append(it->second); // Append encoded string
            } 
            else {
                encoded.push_back(c); // Append character directly
            }
        }
        return encoded;
    }

    /**
     * @brief Decodes a percent-encoded string.
     * @param str The percent-encoded string to decode.
     * @return The decoded string.
     * @note If the percent-encoded sequence is invalid (not followed by two hexadecimal digits),
     * the '%' character is preserved in the output.
     */
    inline std::string decode(const std::string& str) noexcept {
        std::string decoded;
        size_t i = 0;

        while(i < str.size()) {
            if((str[i] == '%') && (i + 2 < str.size())) {
                // Extract the next two characters after '%' as a hex string
                std::string hex = str.substr(i + 1, 2);
                try {
                    // Convert hex string to a character and append to decoded string
                    decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
                    i += 2;  // Skip next two characters
                } 
                catch(...) { // Catch all exceptions
                    // If conversion fails, append '%' and continue
                    decoded += "%"; 
                }
            }
            else {
                // Append character directly if it's not part of a percent-encoded sequence
                decoded += str[i];
            }
            i++;
        }
        return decoded;
    }
}

#endif // HTTP_ENCODING_HPP