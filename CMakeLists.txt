cmake_minimum_required(VERSION 3.12)
project(scat VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(scat
    ${SOURCES}
)

# Проверяем, нужно ли линковать stdc++fs (старые gcc/libstdc++)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-lstdc++fs" HAS_LIBSTDCXXFS)

if (HAS_LIBSTDCXXFS)
    target_link_libraries(scat stdc++fs)
endif()

# ---------- INSTALL ----------

# Установка бинарника в стандартный путь
install(
    TARGETS scat
    RUNTIME DESTINATION bin
)

# Пример man-страницы: если захочешь — просто создай файл docs/scat.1
if (EXISTS "${CMAKE_SOURCE_DIR}/docs/scat.1")
    install(
        FILES docs/scat.1
        DESTINATION share/man/man1
    )
endif()