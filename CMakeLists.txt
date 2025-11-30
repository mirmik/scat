cmake_minimum_required(VERSION 3.12)
project(scat VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

enable_testing()

if (MSVC)
    add_compile_options(/permissive- /W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# -----------------------------------------------------------------------------
# Platform-specific install prefixes
# -----------------------------------------------------------------------------
set(SCAT_ON_TERMUX FALSE)

if (DEFINED ENV{TERMUX_VERSION} OR CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(SCAT_ON_TERMUX TRUE)
endif()

if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}/Programs/scat"
        CACHE PATH "Install path" FORCE)
elseif (SCAT_ON_TERMUX AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Respect Termux prefix so `cmake --install` ends up under $PREFIX
    set(CMAKE_INSTALL_PREFIX "$ENV{PREFIX}"
        CACHE PATH "Install path" FORCE)
endif()

# -----------------------------------------------------------------------------
# Gather sources with auto-update
# -----------------------------------------------------------------------------
file(GLOB SCAT_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# main.cpp не должен входить в логику тестов
list(REMOVE_ITEM SCAT_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

file(GLOB SCAT_HEADERS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# -----------------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------------
add_executable(scat
    src/main.cpp        # тонкая обёртка
    ${SCAT_SOURCES}     # scat.cpp
    ${SCAT_HEADERS}     # scat.h
)

# std::filesystem fix for MinGW and older libstdc++ (detect actual linker support)
# Prefer the standard library's builtin support; fall back to stdc++fs only when needed
check_cxx_source_compiles(
    [[#include <filesystem>
      int main() { std::filesystem::path p{"."}; return p.empty(); }]]
    HAS_BUILTIN_FILESYSTEM
)

if (NOT HAS_BUILTIN_FILESYSTEM)
    set(CMAKE_REQUIRED_LIBRARIES "stdc++fs")
    check_cxx_source_compiles(
        [[#include <filesystem>
          int main() { std::filesystem::path p{"."}; return p.empty(); }]]
        HAS_LIBSTDCXXFS
    )
    unset(CMAKE_REQUIRED_LIBRARIES)
endif()

set(FILESYSTEM_LIB "")
if (HAS_LIBSTDCXXFS)
    set(FILESYSTEM_LIB stdc++fs)
endif()

if (FILESYSTEM_LIB)
    target_link_libraries(scat PRIVATE ${FILESYSTEM_LIB})
endif()

# -----------------------------------------------------------------------------
# Install executable
# -----------------------------------------------------------------------------
install(TARGETS scat
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# -----------------------------------------------------------------------------
# Windows shim (scat.cmd in WindowsApps)
# -----------------------------------------------------------------------------
if (WIN32)
    install(CODE "
        set(wapps \"\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\")
        file(MAKE_DIRECTORY \"\${wapps}\")
        set(scat_path \"\$ENV{LOCALAPPDATA}/Programs/scat/${CMAKE_INSTALL_BINDIR}/scat.exe\")
if(NOT EXISTS \"\${wapps}/scat.cmd\")
    file(WRITE \"\${wapps}/scat.cmd\" \"@echo off\n\\\"\${scat_path}\\\" %*\")
endif()
    ")
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
file(GLOB SCAT_TESTS CONFIGURE_DEPENDS tests/*)

add_executable(scat_tests
    ${SCAT_TESTS}
    ${SCAT_SOURCES}
    ${SCAT_HEADERS}
)

target_include_directories(scat_tests PRIVATE src tests)

if (FILESYSTEM_LIB)
    target_link_libraries(scat_tests PRIVATE ${FILESYSTEM_LIB})
endif()

if (WIN32)
    target_link_libraries(scat_tests ws2_32)
endif()

add_test(NAME scat_tests COMMAND scat_tests)
