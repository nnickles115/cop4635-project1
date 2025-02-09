/**
 * @file http_mime.hpp
 * @brief This file contains constants and utility functions for 
 * working with MIME types.
 * 
 * @author Noah Nickles
 * @date 1/30/2025
 * COP4635 Sys & Net II - Project 1
 */

#ifndef HTTP_MIME_HPP
#define HTTP_MIME_HPP

#include <string>
#include <string_view>
#include <unordered_map>

namespace http::mime {
    enum class Media {
        // Application
        APP_FORM,
        APP_JSON,
        APP_JAVASCRIPT,
        APP_OCTET_STREAM,
        APP_XML,
        APP_ZIP,

        // Audio
        AUDIO_MPEG,
        AUDIO_OGG,
        AUDIO_WAV,

        // Font
        FONT_OTF,
        FONT_TTF,
        FONT_WOFF,
        FONT_WOFF2,

        // Image
        IMAGE_GIF,
        IMAGE_JPEG,
        IMAGE_PNG,
        IMAGE_SVG_XML,
        IMAGE_WEBP,

        // Text
        TEXT_CSS,
        TEXT_CSV,
        TEXT_HTML,
        TEXT_PLAIN,
        TEXT_XML,

        // Video
        VIDEO_MP4,
        VIDEO_MPEG,
        VIDEO_OGG,
        VIDEO_WEBM,

        // Error or Unknown
        INVALID
    };
}

namespace std {
    /**
     * @brief The hash specialization for the http::mime::Media enum class.
     * @details This allows the enum value to be used as a key in an unordered_map due
     * to it being an enum class.
     * @remark Normal enums leak their values into the surrounding scope, which can cause
     * collisions in the hash map, so this is a safer alternative.
     * @note Hash functions have to be defined in the std namespace.
     */
    template<> struct hash<http::mime::Media> {
        size_t operator()(const http::mime::Media& c) const noexcept {
            return static_cast<size_t>(c);
        }
    };
}

namespace http::mime {
    inline const std::unordered_map<Media, std::string_view> MIME_MAP {
        {Media::APP_FORM,         "application/x-www-form-urlencoded"},
        {Media::APP_JAVASCRIPT,   "application/javascript"},
        {Media::APP_JSON,         "application/json"},
        {Media::APP_OCTET_STREAM, "application/octet-stream"},
        {Media::APP_XML,          "application/xml"},
        {Media::APP_ZIP,          "application/zip"},
        {Media::AUDIO_MPEG,       "audio/mpeg"},
        {Media::AUDIO_OGG,        "audio/ogg"},
        {Media::AUDIO_WAV,        "audio/wav"},
        {Media::FONT_OTF,         "font/otf"},
        {Media::FONT_TTF,         "font/ttf"},
        {Media::FONT_WOFF,        "font/woff"},
        {Media::FONT_WOFF2,       "font/woff2"},
        {Media::IMAGE_GIF,        "image/gif"},
        {Media::IMAGE_JPEG,       "image/jpeg"},
        {Media::IMAGE_PNG,        "image/png"},
        {Media::IMAGE_SVG_XML,    "image/svg+xml"},
        {Media::IMAGE_WEBP,       "image/webp"},
        {Media::TEXT_CSS,         "text/css"},
        {Media::TEXT_CSV,         "text/csv"},
        {Media::TEXT_HTML,        "text/html"},
        {Media::TEXT_PLAIN,       "text/plain"},
        {Media::TEXT_XML,         "text/xml"},
        {Media::VIDEO_MP4,        "video/mp4"},
        {Media::VIDEO_MPEG,       "video/mpeg"},
        {Media::VIDEO_OGG,        "video/ogg"},
        {Media::VIDEO_WEBM,       "video/webm"},
        {Media::INVALID,          "Invalid"}
    };

    inline const std::unordered_map<std::string, Media> EXTENSION_MAP {
        {".bin",   Media::APP_OCTET_STREAM},
        {".css",   Media::TEXT_CSS},
        {".csv",   Media::TEXT_CSV},
        {".exe",   Media::APP_OCTET_STREAM},
        {".gif",   Media::IMAGE_GIF},
        {".htm",   Media::TEXT_HTML},
        {".html",  Media::TEXT_HTML},
        {".jpeg",  Media::IMAGE_JPEG},
        {".jpg",   Media::IMAGE_JPEG},
        {".js",    Media::APP_JAVASCRIPT},
        {".json",  Media::APP_JSON},
        {".mp3",   Media::AUDIO_MPEG},
        {".mp4",   Media::VIDEO_MP4},
        {".mpeg",  Media::VIDEO_MPEG},
        {".ogg",   Media::AUDIO_OGG},
        {".ogv",   Media::VIDEO_OGG},
        {".otf",   Media::FONT_OTF},
        {".png",   Media::IMAGE_PNG},
        {".svg",   Media::IMAGE_SVG_XML},
        {".ttf",   Media::FONT_TTF},
        {".txt",   Media::TEXT_PLAIN},
        {".wav",   Media::AUDIO_WAV},
        {".webm",  Media::VIDEO_WEBM},
        {".webp",  Media::IMAGE_WEBP},
        {".woff",  Media::FONT_WOFF},
        {".woff2", Media::FONT_WOFF2},
        {".xml",   Media::TEXT_XML},
        {".zip",   Media::APP_ZIP}
    };

    /**
     * @brief Converts a MIME type to a string.
     * @param mime The MIME type to convert.
     * @return The string representation of the MIME type.
     */
    inline std::string toString(Media mime) noexcept {
        auto it = MIME_MAP.find(mime);
        return (it != MIME_MAP.end()) ? std::string(it->second) : "Invalid";
    }

    /**
     * @brief Converts a file extension to a MIME type.
     * @param extension The file extension to convert.
     * @return The MIME type.
     */
    inline Media fromExtension(const std::string_view extension) noexcept {
        auto it = EXTENSION_MAP.find(std::string(extension));
        return (it != EXTENSION_MAP.end()) ? it->second : Media::INVALID;
    }

    inline std::string extractMimeType(std::string_view fullType) {
        // Find the semicolon that may separate the charset or other params
        size_t semi = fullType.find(";");
        if(semi != std::string_view::npos) {
            // Remove trailing whitespace if needed
            return std::string(fullType.substr(0, semi));
        }
        return std::string(fullType);
    }
}

#endif // HTTP_MIME_HPP