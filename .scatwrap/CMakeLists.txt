<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CMakeLists.txt</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
cmake_minimum_required(VERSION 3.12)<br>
project(scat VERSION 1.0 LANGUAGES CXX)<br>
<br>
set(CMAKE_CXX_STANDARD 17)<br>
set(CMAKE_CXX_STANDARD_REQUIRED ON)<br>
<br>
# -----------------------------------------------------------------------------<br>
# Windows install prefix<br>
# -----------------------------------------------------------------------------<br>
if (WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)<br>
&#9;set(CMAKE_INSTALL_PREFIX &quot;$ENV{LOCALAPPDATA}/Programs/scat&quot;<br>
&#9;&#9;CACHE PATH &quot;Install path&quot; FORCE)<br>
endif()<br>
<br>
# -----------------------------------------------------------------------------<br>
# Gather sources with auto-update<br>
# -----------------------------------------------------------------------------<br>
file(GLOB SCAT_SOURCES<br>
&#9;CONFIGURE_DEPENDS<br>
&#9;${CMAKE_SOURCE_DIR}/src/*.cpp<br>
)<br>
<br>
# main.cpp не должен входить в логику тестов<br>
list(REMOVE_ITEM SCAT_SOURCES &quot;${CMAKE_SOURCE_DIR}/src/main.cpp&quot;)<br>
<br>
file(GLOB SCAT_HEADERS<br>
&#9;CONFIGURE_DEPENDS<br>
&#9;${CMAKE_SOURCE_DIR}/src/*.h<br>
)<br>
<br>
# -----------------------------------------------------------------------------<br>
# Executable<br>
# -----------------------------------------------------------------------------<br>
add_executable(scat<br>
&#9;src/main.cpp        # тонкая обёртка<br>
&#9;${SCAT_SOURCES}     # scat.cpp<br>
&#9;${SCAT_HEADERS}     # scat.h<br>
)<br>
<br>
# std::filesystem fix for MinGW<br>
include(CheckCXXCompilerFlag)<br>
check_cxx_compiler_flag(&quot;-lstdc++fs&quot; HAS_LIBSTDCXXFS)<br>
if (HAS_LIBSTDCXXFS)<br>
&#9;target_link_libraries(scat stdc++fs)<br>
endif()<br>
<br>
# -----------------------------------------------------------------------------<br>
# Install executable<br>
# -----------------------------------------------------------------------------<br>
install(TARGETS scat<br>
&#9;RUNTIME DESTINATION bin<br>
)<br>
<br>
# -----------------------------------------------------------------------------<br>
# Windows shim (scat.cmd in WindowsApps)<br>
# -----------------------------------------------------------------------------<br>
if (WIN32)<br>
&#9;install(CODE &quot;<br>
&#9;&#9;set(wapps \&quot;\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\&quot;)<br>
&#9;&#9;file(MAKE_DIRECTORY \&quot;\${wapps}\&quot;)<br>
&#9;&#9;set(scat_path \&quot;\$ENV{LOCALAPPDATA}/Programs/scat/bin/scat.exe\&quot;)<br>
if(NOT EXISTS \&quot;\${wapps}/scat.cmd\&quot;)<br>
&#9;file(WRITE \&quot;\${wapps}/scat.cmd\&quot; \&quot;@echo off\n\\\&quot;\${scat_path}\\\&quot; %*\&quot;)<br>
endif()<br>
&#9;&quot;)<br>
endif()<br>
<br>
# -----------------------------------------------------------------------------<br>
# Tests<br>
# -----------------------------------------------------------------------------<br>
enable_testing()<br>
<br>
file(GLOB SCAT_TESTS CONFIGURE_DEPENDS tests/*)<br>
<br>
add_executable(scat_tests<br>
&#9;${SCAT_TESTS}<br>
&#9;${SCAT_SOURCES}   <br>
&#9;${SCAT_HEADERS}   <br>
)<br>
<br>
target_include_directories(scat_tests PRIVATE src tests)<br>
<br>
if (WIN32)<br>
&#9;target_link_libraries(scat_tests ws2_32)<br>
endif()<br>
<br>
add_test(NAME scat_tests COMMAND scat_tests)<br>
<!-- END SCAT CODE -->
</body>
</html>
