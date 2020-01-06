cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Ennov VERSION 0.1.0 LANGUAGES C CXX)

add_executable(EnnovSandbox src/linux_main.cpp)
add_library(Ennov SHARED src/ennov.cpp)

target_link_libraries(EnnovSandbox PUBLIC X11 GL GLX dl)
target_include_directories(EnnovSandbox PUBLIC src/)
