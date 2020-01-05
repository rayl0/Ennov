cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Ennov VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Ennov src/linux_main.cpp)

target_link_libraries(Ennov PUBLIC X11)
