<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CMakeLists.txt</title>
</head>
<body>
<pre><code>
cmake_minimum_required(VERSION 3.12)
project(scat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Windows install prefix
# -----------------------------------------------------------------------------
if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX &quot;$ENV{LOCALAPPDATA}/Programs/scat&quot;
        CACHE PATH &quot;Install path&quot; FORCE)
endif()

# -----------------------------------------------------------------------------
# Gather sources with auto-update
# -----------------------------------------------------------------------------
file(GLOB SCAT_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# main.cpp не должен входить в логику тестов
list(REMOVE_ITEM SCAT_SOURCES &quot;${CMAKE_SOURCE_DIR}/src/main.cpp&quot;)

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
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(&quot;-lstdc++fs&quot; HAS_LIBSTDCXXFS)
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
    install(CODE &quot;
        set(wapps \&quot;\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\&quot;)
        file(MAKE_DIRECTORY \&quot;\${wapps}\&quot;)
        set(scat_path \&quot;\$ENV{LOCALAPPDATA}/Programs/scat/bin/scat.exe\&quot;)
if(NOT EXISTS \&quot;\${wapps}/scat.cmd\&quot;)
    file(WRITE \&quot;\${wapps}/scat.cmd\&quot; \&quot;@echo off\n\\\&quot;\${scat_path}\\\&quot; %*\&quot;)
endif()
    &quot;)
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
enable_testing()

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

</code></pre>
</body>
</html>
