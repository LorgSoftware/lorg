cmake_minimum_required(VERSION 3.10)

project(Lorg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("Using GNU compiler: setting strong requirements for building.")
    add_compile_options(-Wall -Wextra -pedantic)
else()
    message("Compiler used: ${CMAKE_CXX_COMPILER_ID}")
    message("No extra options added.")
endif()

set(LORG_SOURCES
    src/main.cpp
)
add_executable(lorg ${LORG_SOURCES})
