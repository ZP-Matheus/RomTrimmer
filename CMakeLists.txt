cmake_minimum_required(VERSION 3.15)

project(romtrimmerpp
    VERSION 1.0
    LANGUAGES C CXX
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
# Dependências externas
# ===============================
find_package(OpenSSL REQUIRED)

# ===============================
# ZLIB (vendorizada)
# ===============================
add_library(zlib STATIC
    third_party/zlib/adler32.c
    third_party/zlib/crc32.c
    third_party/zlib/deflate.c
    third_party/zlib/inffast.c
    third_party/zlib/inflate.c
    third_party/zlib/inftrees.c
    third_party/zlib/trees.c
    third_party/zlib/zutil.c
)

target_include_directories(zlib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/third_party/zlib
)

target_compile_definitions(zlib PRIVATE ZLIB_CONST)

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
        zlib
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(romtrimmer_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WITH_UCON64_INTEGRATION)
    target_compile_definitions(romtrimmer_core PRIVATE WITH_UCON64=1)
    target_sources(romtrimmer_core PRIVATE src/Ucon64Integration.cpp)
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

    target_include_directories(romtrimmer_c
        PUBLIC
            ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(romtrimmer_c
        PRIVATE
            romtrimmer_core
            zlib
    )

    install(FILES include/romtrimmer_c.h DESTINATION include)
    install(TARGETS romtrimmer_c DESTINATION lib)
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

    add_test(NAME basic_tests COMMAND romtrimmer_tests)
endif()
