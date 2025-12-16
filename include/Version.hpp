/**
 * @file Version.hpp
 * @brief Version information for RomTrimmer++
 *
 * Centralized versioning, build info, feature detection and compatibility helpers.
 */

#pragma once

#include <string>
#include <sstream>
#include <cstdint>

// ==================== VERSION MACROS ====================
#define ROMTRIMMER_VERSION_MAJOR 1
#define ROMTRIMMER_VERSION_MINOR 0
#define ROMTRIMMER_VERSION_PATCH 0
#define ROMTRIMMER_VERSION_TWEAK 0

#define ROMTRIMMER_VERSION_STRING "1.0.0"

// Build information
#define ROMTRIMMER_BUILD_DATE __DATE__
#define ROMTRIMMER_BUILD_TIME __TIME__

// Version as a single integer (MMmmpptt)
#define ROMTRIMMER_VERSION_INT \
    ((ROMTRIMMER_VERSION_MAJOR << 24) | \
     (ROMTRIMMER_VERSION_MINOR << 16) | \
     (ROMTRIMMER_VERSION_PATCH << 8)  | \
     (ROMTRIMMER_VERSION_TWEAK))

// Compatibility aliases (legacy-safe)
#define VERSION_MAJOR  ROMTRIMMER_VERSION_MAJOR
#define VERSION_MINOR  ROMTRIMMER_VERSION_MINOR
#define VERSION_PATCH  ROMTRIMMER_VERSION_PATCH
#define VERSION_STRING ROMTRIMMER_VERSION_STRING
#define BUILD_DATE     ROMTRIMMER_BUILD_DATE
#define BUILD_TIME     ROMTRIMMER_BUILD_TIME

// ==================== FEATURE DETECTION ====================
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

// ==================== PLATFORM DETECTION ====================
#ifdef _WIN32
    #define ROMTRIMMER_PLATFORM_WINDOWS 1
    #define ROMTRIMMER_PLATFORM_LINUX   0
    #define ROMTRIMMER_PLATFORM_MACOS   0
#elif defined(__linux__)
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX   1
    #define ROMTRIMMER_PLATFORM_MACOS   0
#elif defined(__APPLE__)
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX   0
    #define ROMTRIMMER_PLATFORM_MACOS   1
#else
    #define ROMTRIMMER_PLATFORM_WINDOWS 0
    #define ROMTRIMMER_PLATFORM_LINUX   0
    #define ROMTRIMMER_PLATFORM_MACOS   0
#endif

// ==================== ENDIANNESS DETECTION ====================
#if ROMTRIMMER_HAS_CXX20
    #include <bit>
    #if std::endian::native == std::endian::little
        #define ROMTRIMMER_LITTLE_ENDIAN 1
        #define ROMTRIMMER_BIG_ENDIAN    0
    #else
        #define ROMTRIMMER_LITTLE_ENDIAN 0
        #define ROMTRIMMER_BIG_ENDIAN    1
    #endif
#else
    // Conservative fallback
    #if defined(_WIN32)
        #define ROMTRIMMER_LITTLE_ENDIAN 1
        #define ROMTRIMMER_BIG_ENDIAN    0
    #else
        #error "Endianness detection requires C++20 or a platform-specific implementation"
    #endif
#endif

// ==================== DEPRECATION MACROS ====================
#ifdef __GNUC__
    #define ROMTRIMMER_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define ROMTRIMMER_DEPRECATED __declspec(deprecated)
#else
    #define ROMTRIMMER_DEPRECATED
#endif

// ==================== API VERSIONING ====================
#define ROMTRIMMER_API_VERSION 1

#define ROMTRIMMER_CHECK_API_VERSION(version) \
    static_assert((version) <= ROMTRIMMER_API_VERSION, \
        "API version mismatch: required version is higher than available")

// ==================== VERSION NAMESPACE ====================
namespace Version {

inline std::string getVersionString() {
    return ROMTRIMMER_VERSION_STRING;
}

inline void getVersionComponents(int& major, int& minor, int& patch, int& tweak) {
    major = ROMTRIMMER_VERSION_MAJOR;
    minor = ROMTRIMMER_VERSION_MINOR;
    patch = ROMTRIMMER_VERSION_PATCH;
    tweak = ROMTRIMMER_VERSION_TWEAK;
}

inline std::string getBuildInfo() {
    std::stringstream ss;
    ss << "Build: " << ROMTRIMMER_BUILD_DATE << " " << ROMTRIMMER_BUILD_TIME;
    return ss.str();
}

inline uint32_t getVersionInt() {
    return ROMTRIMMER_VERSION_INT;
}

inline bool isCompatible(int requiredMajor, int requiredMinor = 0) {
    if (ROMTRIMMER_VERSION_MAJOR > requiredMajor) return true;
    if (ROMTRIMMER_VERSION_MAJOR == requiredMajor &&
        ROMTRIMMER_VERSION_MINOR >= requiredMinor) return true;
    return false;
}

inline int compare(int major, int minor = 0, int patch = 0) {
    if (ROMTRIMMER_VERSION_MAJOR != major)
        return (ROMTRIMMER_VERSION_MAJOR < major) ? -1 : 1;
    if (ROMTRIMMER_VERSION_MINOR != minor)
        return (ROMTRIMMER_VERSION_MINOR < minor) ? -1 : 1;
    if (ROMTRIMMER_VERSION_PATCH != patch)
        return (ROMTRIMMER_VERSION_PATCH < patch) ? -1 : 1;
    return 0;
}

inline std::string getFullVersionInfo() {
    std::stringstream ss;
    ss << "RomTrimmer++ v" << ROMTRIMMER_VERSION_STRING << "\n"
       << "Build: " << ROMTRIMMER_BUILD_DATE << " " << ROMTRIMMER_BUILD_TIME << "\n"
       << "ABI: " << ROMTRIMMER_VERSION_INT;
    return ss.str();
}

inline std::string getLicense() {
    return "MIT License - See LICENSE file for details.";
}

inline std::string getSupportInfo() {
    return
        "Repository: https://github.com/ZP-Matheus/romtrimmer\n"
        "Issues: https://github.com/ZP-Matheus/romtrimmer/issues";
}

} // namespace Version