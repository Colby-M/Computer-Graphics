cmake_minimum_required(VERSION 3.21)
project(Computer_Graphics)

set(CMAKE_CXX_STANDARD 14)

include_directories(glad)

# Including GLFW
add_subdirectory(glfw)
include_directories(glfw/include)


include_directories(texture.frag)
include_directories(texture.vert)

set(SOURCE_FILES Project42022.cpp)

add_executable(Computer_Graphics
        glad/glad.c
        LoadShaders.cpp
        objReader.cpp
        Project42022.cpp
        linmath.h
        )
# Linking GLFW
target_link_libraries(Computer_Graphics glfw)
