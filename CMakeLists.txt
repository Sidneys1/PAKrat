cmake_minimum_required(VERSION 3.20)
project(sspyth VERSION 0.1 DESCRIPTION "Reverse engineering of Math Invaders.")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pakrat pakrat.cpp)
# add_subdirectory(pakrat)
set_target_properties(pakrat PROPERTIES CMAKE_CXX_STANDARD 20)
set_target_properties(pakrat PROPERTIES CMAKE_CXX_EXTENSIONS OFF)
set_target_properties(pakrat PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)