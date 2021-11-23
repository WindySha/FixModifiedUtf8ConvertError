cmake_minimum_required(VERSION 3.10.2)

project("test-lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(test-lib
        SHARED
        main.cpp
        )

find_library(log-lib
        log)

target_link_libraries(test-lib
        ${log-lib})