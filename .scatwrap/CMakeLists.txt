<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CMakeLists.txt</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
cmake_minimum_required(VERSION &#20;3.12)<br>
project(scat &#20;VERSION &#20;1.0 &#20;LANGUAGES &#20;CXX)<br>
<br>
set(CMAKE_CXX_STANDARD &#20;17)<br>
set(CMAKE_CXX_STANDARD_REQUIRED &#20;ON)<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Windows &#20;install &#20;prefix<br>
# &#20;-----------------------------------------------------------------------------<br>
if &#20;(WIN32 &#20;AND &#20;CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)<br>
 &#20; &#20; &#20; &#20;set(CMAKE_INSTALL_PREFIX &#20;&quot;$ENV{LOCALAPPDATA}/Programs/scat&quot;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;CACHE &#20;PATH &#20;&quot;Install &#20;path&quot; &#20;FORCE)<br>
endif()<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Gather &#20;sources &#20;with &#20;auto-update<br>
# &#20;-----------------------------------------------------------------------------<br>
file(GLOB &#20;SCAT_SOURCES<br>
 &#20; &#20; &#20; &#20;CONFIGURE_DEPENDS<br>
 &#20; &#20; &#20; &#20;${CMAKE_SOURCE_DIR}/src/*.cpp<br>
)<br>
<br>
# &#20;main.cpp &#20;не &#20;должен &#20;входить &#20;в &#20;логику &#20;тестов<br>
list(REMOVE_ITEM &#20;SCAT_SOURCES &#20;&quot;${CMAKE_SOURCE_DIR}/src/main.cpp&quot;)<br>
<br>
file(GLOB &#20;SCAT_HEADERS<br>
 &#20; &#20; &#20; &#20;CONFIGURE_DEPENDS<br>
 &#20; &#20; &#20; &#20;${CMAKE_SOURCE_DIR}/src/*.h<br>
)<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Executable<br>
# &#20;-----------------------------------------------------------------------------<br>
add_executable(scat<br>
 &#20; &#20; &#20; &#20;src/main.cpp &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;# &#20;тонкая &#20;обёртка<br>
 &#20; &#20; &#20; &#20;${SCAT_SOURCES} &#20; &#20; &#20; &#20; &#20;# &#20;scat.cpp<br>
 &#20; &#20; &#20; &#20;${SCAT_HEADERS} &#20; &#20; &#20; &#20; &#20;# &#20;scat.h<br>
)<br>
<br>
# &#20;std::filesystem &#20;fix &#20;for &#20;MinGW<br>
include(CheckCXXCompilerFlag)<br>
check_cxx_compiler_flag(&quot;-lstdc++fs&quot; &#20;HAS_LIBSTDCXXFS)<br>
if &#20;(HAS_LIBSTDCXXFS)<br>
 &#20; &#20; &#20; &#20;target_link_libraries(scat &#20;stdc++fs)<br>
endif()<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Install &#20;executable<br>
# &#20;-----------------------------------------------------------------------------<br>
install(TARGETS &#20;scat<br>
 &#20; &#20; &#20; &#20;RUNTIME &#20;DESTINATION &#20;bin<br>
)<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Windows &#20;shim &#20;(scat.cmd &#20;in &#20;WindowsApps)<br>
# &#20;-----------------------------------------------------------------------------<br>
if &#20;(WIN32)<br>
 &#20; &#20; &#20; &#20;install(CODE &#20;&quot;<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;set(wapps &#20;\&quot;\$ENV{LOCALAPPDATA}/Microsoft/WindowsApps\&quot;)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;file(MAKE_DIRECTORY &#20;\&quot;\${wapps}\&quot;)<br>
 &#20; &#20; &#20; &#20; &#20; &#20; &#20; &#20;set(scat_path &#20;\&quot;\$ENV{LOCALAPPDATA}/Programs/scat/bin/scat.exe\&quot;)<br>
if(NOT &#20;EXISTS &#20;\&quot;\${wapps}/scat.cmd\&quot;)<br>
 &#20; &#20; &#20; &#20;file(WRITE &#20;\&quot;\${wapps}/scat.cmd\&quot; &#20;\&quot;@echo &#20;off\n\\\&quot;\${scat_path}\\\&quot; &#20;%*\&quot;)<br>
endif()<br>
 &#20; &#20; &#20; &#20;&quot;)<br>
endif()<br>
<br>
# &#20;-----------------------------------------------------------------------------<br>
# &#20;Tests<br>
# &#20;-----------------------------------------------------------------------------<br>
enable_testing()<br>
<br>
file(GLOB &#20;SCAT_TESTS &#20;CONFIGURE_DEPENDS &#20;tests/*)<br>
<br>
add_executable(scat_tests<br>
 &#20; &#20; &#20; &#20;${SCAT_TESTS}<br>
 &#20; &#20; &#20; &#20;${SCAT_SOURCES} &#20; &#20; &#20;<br>
 &#20; &#20; &#20; &#20;${SCAT_HEADERS} &#20; &#20; &#20;<br>
)<br>
<br>
target_include_directories(scat_tests &#20;PRIVATE &#20;src &#20;tests)<br>
<br>
if &#20;(WIN32)<br>
 &#20; &#20; &#20; &#20;target_link_libraries(scat_tests &#20;ws2_32)<br>
endif()<br>
<br>
add_test(NAME &#20;scat_tests &#20;COMMAND &#20;scat_tests)<br>
<!-- END SCAT CODE -->
</body>
</html>
