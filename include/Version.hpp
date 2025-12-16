/**
 * @file Version.hpp
 * @brief Version information for RomTrimmer++
 * 
 * This file contains version macros and functions for the RomTrimmer++ project.
 * It provides compile-time version information and runtime version checking.
 */

#pragma once

#include <string>
#include <sstream>

// ==================== VERSION MACROS ====================
#define ROMTRIMMER_VERSION_MAJOR 1
#define ROMTRIMMER_VERSION_MINOR 0
#define ROMTRIMMER_VERSION_PATCH 0
#define ROMTRIMMER_VERSION_TWEAK 0

// String representation
#define ROMTRIMMER_VERSION_STRING "1.0.0"

// Build information
#define ROMTRIMMER_BUILD_DATE __DATE__
#define ROMTRIMMER_BUILD_TIME __TIME__

// Version as a single integer (MMmmpptt)
#define ROMTRIMMER_VERSION_INT \
    ((ROMTRIMMER_VERSION_MAJOR << 24) | \
     (ROMTRIMMER_VERSION_MINOR << 16) | \
     (ROMTRIMMER_VERSION_PATCH << 8) | \
     ROMTRIMMER_VERSION_TWEAK)

// Compatibility macros
#define VERSION_MAJOR ROMTRIMMER_VERSION_MAJOR
#define VERSION_MINOR ROMTRIMMER_VERSION_MINOR
#define VERSION_PATCH ROMTRIMMER_VERSION_PATCH
#define VERSION_STRING ROMTRIMMER_VERSION_STRING
#define BUILD_DATE ROMTRIMMER_BUILD_DATE
#define BUILD_TIME ROMTRIMMER_BUILD_TIME

// ==================== VERSION NAMESPACE ====================
namespace Version {
    
    /**
     * @brief Get the complete version string
     * @return Version string in format "major.minor.patch"
     */
    inline std::string getVersionString() {
        return ROMTRIMMER_VERSION_STRING;
    }
    
    /**
     * @brief Get version as separate components
     * @param[out] major Major version number
     * @param[out] minor Minor version number
     * @param[out] patch Patch version number
     * @param[out] tweak Tweak version number
     */
    inline void getVersionComponents(int& major, int& minor, int& patch, int& tweak) {
        major = ROMTRIMMER_VERSION_MAJOR;
        minor = ROMTRIMMER_VERSION_MINOR;
        patch = ROMTRIMMER_VERSION_PATCH;
        tweak = ROMTRIMMER_VERSION_TWEAK;
    }
    
    /**
     * @brief Get the build date and time
     * @return String containing build date and time
     */
    inline std::string getBuildInfo() {
        std::stringstream ss;
        ss << "Build: " << ROMTRIMMER_BUILD_DATE << " " << ROMTRIMMER_BUILD_TIME;
        return ss.str();
    }
    
    /**
     * @brief Get version as a single integer
     * @return Version as 32-bit integer (MMmmpptt)
     */
    inline uint32_t getVersionInt() {
        return ROMTRIMMER_VERSION_INT;
    }
    
    /**
     * @brief Check if current version is compatible with required version
     * @param requiredMajor Required major version
     * @param requiredMinor Required minor version
     * @return True if version is compatible
     */
    inline bool isCompatible(int requiredMajor, int requiredMinor = 0) {
        if (ROMTRIMMER_VERSION_MAJOR > requiredMajor) return true;
        if (ROMTRIMMER_VERSION_MAJOR == requiredMajor && 
            ROMTRIMMER_VERSION_MINOR >= requiredMinor) return true;
        return false;
    }
    
    /**
     * @brief Compare versions
     * @param major Version major component
     * @param minor Version minor component
     * @param patch Version patch component
     * @return -1 if current < specified, 0 if equal, 1 if current > specified
     */
    inline int compare(int major, int minor = 0, int patch = 0) {
        if (ROMTRIMMER_VERSION_MAJOR < major) return -1;
        if (ROMTRIMMER_VERSION_MAJOR > major) return 1;
        
        if (ROMTRIMMER_VERSION_MINOR < minor) return -1;
        if (ROMTRIMMER_VERSION_MINOR > minor) return 1;
        
        if (ROMTRIMMER_VERSION_PATCH < patch) return -1;
        if (ROMTRIMMER_VERSION_PATCH > patch) return 1;
        
        return 0;
    }
    
    /**
     * @brief Get complete version information
     * @return Formatted string with all version details
     */
    inline std::string getFullVersionInfo() {
        std::stringstream ss;
        ss << "RomTrimmer++ v" << ROMTRIMMER_VERSION_STRING << "\n"
           << "Version: " << ROMTRIMMER_VERSION_MAJOR << "." 
                          << ROMTRIMMER_VERSION_MINOR << "." 
                          << ROMTRIMMER_VERSION_PATCH;
        
        if (ROMTRIMMER_VERSION_TWEAK > 0) {
            ss << "." << ROMTRIMMER_VERSION_TWEAK;
        }
        
        ss << "\n"
           << "Build: " << ROMTRIMMER_BUILD_DATE << " " << ROMTRIMMER_BUILD_TIME << "\n"
           << "ABI Version: " << ROMTRIMMER_VERSION_INT;
        
        return ss.str();
    }
    
    /**
     * @brief Get copyright information
     * @return Copyright string
     */
    inline std::string getCopyright() {
        return "Copyright (c) 2024 RomTrimmer++ Project. All rights reserved.";
    }
    
    /**
     * @brief Get license information
     * @return License string
     */
    inline std::string getLicense() {
        return "MIT License - See LICENSE file for details.";
    }
    
    /**
     * @brief Get support information
     * @return Support contact information
     */
    inline std::string getSupportInfo() {
        return "Repository: https://github.com/romtrimmer/romtrimmer-plusplus\n"
               "Issues: https://github.com/romtrimmer/romtrimmer-plusplus/issues";
    }
}

// ==================== DEPRECATION MACROS ====================
#ifdef __GNUC__
    #define ROMTRIMMER_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define ROMTRIMMER_DEPRECATED __declspec(deprecated)
#else
    #define ROMTRIMMER_DEPRECATED
#endif

// ==================== API VERSIONING ====================
// Current API version
#define ROMTRIMMER_API_VERSION 1

// API compatibility check
#define ROMTRIMMER_CHECK_API_VERSION(version) \
    static_assert(version <= ROMTRIMMER_API_VERSION, \
                  "API version mismatch: required version is higher than available")

// ==================== FEATURE DETECTION ====================
// Compiler feature detection
#ifndef ROMTRIMMER_HAS_CXX17
    #if __cplusplus >= 201703L
        #define ROMTRIMMER_HAS_CXX17 1
    #else
        #define ROMTRIMMER_HAS_CXX17 0
    #endif
#endif

#ifndef ROMTRIMMER_HAS_CXX20
    #if __cplusplus >= 202002L
        #define ROMTRIMMER_HAS_CXX20 1
    #else
        #define ROMTRIMMER_HAS_CXX20 0
    #endif
#endif

// Platform detection
#ifdef _WIN32
    #define ROMTRIMMER_PLATFORM_WINDOWS 1
    #define ROMTRIMMER_PLATFORM_LINUX 0
    #define ROMTRIMMER_PLATFORM_MACOS 0
#elif __linux__
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX 1
    #define ROMTRIMMER_PLATFORM_MACOS 0
#elif __APPLE__
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX 0
    #define ROMTRIMMER_PLATFORM_MACOS 1
#else
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX 0
    #define ROMTRIMMER_PLATFORM_MACOS 0
#endif

// Endianness detection
#ifdef _WIN32
    #define ROMTRIMMER_LITTLE_ENDIAN 1
    #define ROMTRIMMER_BIG_ENDIAN 0
#else
    #include <endian.h>
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        #define ROMTRIMMER_LITTLE_ENDIAN 1
        #define ROMTRIMMER_BIG_ENDIAN 0
    #else
        #define ROMTRIMMER_LITTLE_ENDIAN 0
        #define ROMTRIMMER_BIG_ENDIAN 1
    #endif
#endif

// ==================== COMPATIBILITY MACROS ====================
// Ensure compatibility macros are available for older code
#ifndef VERSION_STRING
    #define VERSION_STRING ROMTRIMMER_VERSION_STRING
#endif

#ifndef BUILD_DATE
    #define BUILD_DATE ROMTRIMMER_BUILD_DATE
#endif

#ifndef BUILD_TIME
    #define BUILD_TIME ROMTRIMMER_BUILD_TIME
#endif