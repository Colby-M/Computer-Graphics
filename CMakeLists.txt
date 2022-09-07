cmake_minimum_required(VERSION 3.21)
project(Computer_Graphics)

set(CMAKE_CXX_STANDARD 14)

include_directories(glad)
include_directories(glfw-3.3.8.bin.MACOS)
include_directories(glfw-3.3.8.bin.MACOS/include)
include_directories(glfw-3.3.8.bin.MACOS/include/GLFW)

add_executable(Computer_Graphics
        glad/glad.c
        LoadShaders.cpp
        glfw-3.3.8.bin.MACOS/include/GLFW/glfw3.h
        glfw-3.3.8.bin.MACOS/include/GLFW/glfw3native.h
        glfw-3.3.8.bin.MACOS/lib-x86_64/libglfw3.a
        homework3.cpp
        )
