cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(ennov VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(src/glm EXCLUDE_FROM_ALL)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

add_executable(linux_ennov src/linux_ennov.cpp)
add_library(ennov SHARED src/ennov.cpp)

target_link_libraries(ennov PUBLIC glm::glm)
target_compile_options(ennov PUBLIC -DENNOV_DEBUG -O0)
target_compile_features(ennov PUBLIC c_std_99 cxx_std_11)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set_property(TARGET ennov PROPERTY PREFIX "")
endif()

target_link_libraries(linux_ennov PUBLIC X11 GL GLX pthread dl glm::glm)
target_include_directories(linux_ennov PUBLIC src/)
target_compile_options(linux_ennov PUBLIC -DENNOV_DEBUG -O0)
target_compile_features(linux_ennov PUBLIC c_std_99 cxx_std_98)
