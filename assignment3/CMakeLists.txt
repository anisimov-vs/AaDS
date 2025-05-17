cmake_minimum_required(VERSION 3.10)
project(ShannonFanoTool VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_SOURCES
    src/main.cpp
    src/shannon_fano.cpp
)

set(APP_HEADERS
    include/shannon_fano.hpp
)

add_executable(shannon_fano_tool ${APP_SOURCES} ${APP_HEADERS})

target_include_directories(shannon_fano_tool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(shannon_fano_tool PRIVATE /W4 /WX)
else()
    target_compile_options(shannon_fano_tool PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

message(STATUS "Configuration done. To build, run 'cmake --build .'")