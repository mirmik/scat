cmake_minimum_required(VERSION 3.12)
project(scat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Windows proper installation layout
# -----------------------------------------------------------------------------
if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Install into %LOCALAPPDATA%/Programs/scat
    set(CMAKE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}/Programs/scat"
        CACHE PATH "Install path" FORCE)
endif()

# -----------------------------------------------------------------------------
# Target
# -----------------------------------------------------------------------------
add_executable(scat
    src/main.cpp
)

# Old MinGW compatibility (filesystem)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-lstdc++fs" HAS_LIBSTDCXXFS)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(scat stdc++fs)
endif()

# -----------------------------------------------------------------------------
# Install
# -----------------------------------------------------------------------------
install(TARGETS scat
    RUNTIME DESTINATION bin
)

# Optional manpage for Linux/macOS
if (EXISTS "${CMAKE_SOURCE_DIR}/docs/scat.1")
    install(FILES docs/scat.1
        DESTINATION share/man/man1
    )
endif()

# -----------------------------------------------------------------------------
# Windows shim creation:
# Creates scat.cmd inside WindowsApps so `scat` is globally runnable.
# -----------------------------------------------------------------------------
if (WIN32)
    install(CODE "
        set(wapps \"\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\")
        file(MAKE_DIRECTORY \"\${wapps}\")

        # Use forward slashes to avoid CMake escape issues
        set(scat_path \"\$ENV{LOCALAPPDATA}/Programs/scat/bin/scat.exe\")

        file(WRITE \"\${wapps}/scat.cmd\" \"@echo off\n\\\"\${scat_path}\\\" %*\")
    ")
endif()



# TESTING
enable_testing()

add_executable(scat_tests
    tests/main.cpp
    tests/scat.cpp
)

target_include_directories(scat_tests PRIVATE tests)

add_test(NAME scat_tests COMMAND scat_tests)