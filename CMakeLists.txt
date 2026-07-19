cmake_minimum_required(VERSION 3.16)
project(pubg_mx LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CLangFormat.cmake)
include(cmake/Target.cmake)

set(HIJACK_INSTALL_NAME "/Library/Frameworks/libwebp.framework/libwebp")
set(OUTPUT_BINARY_NAME  "libwebp")

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui EXCLUDE_FROM_ALL)

if(NOT CMAKE_SYSTEM_PROCESSOR)
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
    endif()
endif()

add_subdirectory(SDK)
add_subdirectory(src)
