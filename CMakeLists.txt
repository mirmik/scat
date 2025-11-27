cmake_minimum_required(VERSION 3.12)
project(scat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-lstdc++fs" HAS_LIBSTDCXXFS)

enable_testing()

# -----------------------------------------------------------------------------
# Windows install prefix
# -----------------------------------------------------------------------------
if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}/Programs/scat"
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

# std::filesystem fix for MinGW
if (HAS_LIBSTDCXXFS)
    target_link_libraries(scat stdc++fs)
endif()

# -----------------------------------------------------------------------------
# Install executable
# -----------------------------------------------------------------------------
install(TARGETS scat
    RUNTIME DESTINATION bin
)

# -----------------------------------------------------------------------------
# Windows shim (scat.cmd in WindowsApps)
# -----------------------------------------------------------------------------
if (WIN32)
    install(CODE "
        set(wapps \"\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\")
        file(MAKE_DIRECTORY \"\${wapps}\")
        set(scat_path \"\$ENV{LOCALAPPDATA}/Programs/scat/bin/scat.exe\")
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

if (WIN32)
    target_link_libraries(scat_tests ws2_32)
endif()

add_test(NAME scat_tests COMMAND scat_tests)
