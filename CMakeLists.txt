cmake_minimum_required(VERSION 3.15)

project(romtrimmerpp
    VERSION 1.0
    LANGUAGES CXX
)

# ===============================
# Padrão C++
# ===============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===============================
# Opções de build
# ===============================
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_C_LIBRARY   "Build C interface library" ON)
option(BUILD_CLI_TOOL    "Build CLI tool" ON)
option(BUILD_TESTS       "Build tests" ON)
option(WITH_UCON64_INTEGRATION "Enable uCon64 integration" OFF)

# ===============================
# Dependências
# ===============================
find_package(OpenSSL REQUIRED)

# ===============================
# Core Library
# ===============================
add_library(romtrimmer_core
    src/RomTrimmer.cpp
    src/RomDetector.cpp
    src/PaddingAnalyzer.cpp
    src/SafetyValidator.cpp
    src/ThreadPool.cpp
    src/Logger.cpp
    src/ConfigManager.cpp
    src/LocalizationManager.cpp
)

target_include_directories(romtrimmer_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(romtrimmer_core
    PUBLIC
        OpenSSL::Crypto
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(romtrimmer_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# uCon64 opcional
if(WITH_UCON64_INTEGRATION)
    target_compile_definitions(romtrimmer_core PRIVATE WITH_UCON64=1)
    target_sources(romtrimmer_core PRIVATE src/Ucon64Integration.cpp)
endif()

# ===============================
# CLI Tool
# ===============================
if(BUILD_CLI_TOOL)
    add_executable(romtrimmer++
        src/main.cpp
    )

    target_link_libraries(romtrimmer++
        PRIVATE
            romtrimmer_core
            OpenSSL::SSL
    )

    install(TARGETS romtrimmer++ DESTINATION bin)
endif()

# ===============================
# C Interface Library
# ===============================
if(BUILD_C_LIBRARY)
    add_library(romtrimmer_c SHARED
        src/romtrimmer_c.cpp
        src/ReversePadding.cpp
        src/DatIntegration.cpp
    )

    target_link_libraries(romtrimmer_c
        PRIVATE
            romtrimmer_core
    )

    install(FILES include/romtrimmer_c.h DESTINATION include)
    install(TARGETS romtrimmer_c DESTINATION lib)
endif()

# ===============================
# Testes
# ===============================
if(BUILD_TESTS)
    enable_testing()

    add_executable(romtrimmer_tests
        tests/test_basic.cpp
        src/catch_amalgamated.cpp
    )

    target_link_libraries(romtrimmer_tests
        PRIVATE
            romtrimmer_core
    )

    if(ANDROID)
        target_link_libraries(romtrimmer_tests PRIVATE log)
    endif()

    add_test(NAME basic_tests COMMAND romtrimmer_tests)
endif()
