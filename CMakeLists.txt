cmake_minimum_required(VERSION 3.15)

project(romtrimmerpp
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===============================
# Fontes
# ===============================
set(ROMTRIMMER_SOURCES
    src/main.cpp
    src/RomTrimmer.cpp
    src/RomDetector.cpp
    src/PaddingAnalyzer.cpp
    src/SafetyValidator.cpp
    src/Logger.cpp
    src/ConfigManager.cpp
    src/ThreadPool.cpp
    src/LocalizationManager.cpp
)

add_executable(romtrimmer ${ROMTRIMMER_SOURCES}
    src/main.cpp
    src/RomTrimmer.cpp
    src/ConfigManager.cpp  # <- garantir que está aqui
    src/RomDetector.cpp
    src/PaddingAnalyzer.cpp
    src/SafetyValidator.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LocalizationManager.cpp
)

target_include_directories(romtrimmer
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/external/cxxopts/include
)

find_package(OpenSSL REQUIRED)

target_link_libraries(romtrimmer
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(romtrimmer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ===============================
# Testes
# ===============================
enable_testing()

add_executable(romtrimmer_tests
    tests/test_basic.cpp
    src/RomDetector.cpp
    src/PaddingAnalyzer.cpp
    src/SafetyValidator.cpp
    src/Logger.cpp
    src/LocalizationManager.cpp
    src/catch_amalgamated.cpp
)

target_include_directories(romtrimmer_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(romtrimmer_tests
    PRIVATE
        OpenSSL::Crypto
)

target_link_libraries(romtrimmer_tests PRIVATE log)

add_test(NAME basic_tests COMMAND romtrimmer_tests)

install(TARGETS romtrimmer DESTINATION bin)