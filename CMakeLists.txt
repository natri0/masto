cmake_minimum_required(VERSION 3.28)
project(Masto)

# enable LTO
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
cmake_policy(SET CMP0069 NEW)

# for raylib
set(CUSTOMIZE_BUILD ON)

add_subdirectory(src)
add_subdirectory(raylib)
add_subdirectory(json)
