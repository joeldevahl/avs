cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(avs)

add_executable(avs main.cpp avs.cpp)
