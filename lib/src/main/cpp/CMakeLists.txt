cmake_minimum_required(VERSION 3.10.2)

project("utf8_fixer")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(utf8_fixer
        SHARED
        main.cpp
        func_replace.cpp
        utf.cpp
        elf_util.cpp
        )
find_library(log-lib
        log)
target_link_libraries(
        utf8_fixer
        ${log-lib})
