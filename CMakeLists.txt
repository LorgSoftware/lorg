cmake_minimum_required(VERSION 3.10)

project(Lorg)

set(CMAKE_DEBUG_POSTFIX -debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("Using GNU compiler: setting strong requirements for building.")
    add_compile_options(-Wall -Wextra -pedantic -Wconversion)
else()
    message("Compiler used: ${CMAKE_CXX_COMPILER_ID}")
    message("No extra options added.")
endif()

set(LORG_SOURCES
    src/main.cpp
    src/lorg.cpp
)
add_executable(lorg ${LORG_SOURCES})
set_target_properties(lorg PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
