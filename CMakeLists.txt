cmake_minimum_required(VERSION 3.21)
project(Computer_Graphics)

set(CMAKE_CXX_STANDARD 14)

include_directories(glad)

# Including GLFW
add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(passthrough.frag)
include_directories(passthrough.vert)

set(SOURCE_FILES homework3.cpp)

add_executable(Computer_Graphics
        glad/glad.c
        LoadShaders.cpp
        homework3.cpp
        )
# Linking GLFW
target_link_libraries(Computer_Graphics glfw)
