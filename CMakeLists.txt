cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Ennov VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(src/glm EXCLUDE_FROM_ALL)

add_executable(EnnovSandbox src/linux_ennov.cpp)
add_library(Ennov SHARED src/ennov.cpp)

target_link_libraries(EnnovSandbox PUBLIC X11 GL GLX dl glm::glm)
target_include_directories(EnnovSandbox PUBLIC src/)
