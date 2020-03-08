cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(ennov VERSION 0.1.0 LANGUAGES C CXX)

set(ENNOV_PLATFORM_LINUX 0)
set(ENNOV_LINUX_API_SDL 0)
set(ENNOV_LINUX_API_X11 0)

set(ENNOV_PLATFORM_ANDROID 0)

set(ENNOV_DEBUG 0)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(ENNOV_PLATFORM_LINUX 1)
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
  set(ENNOV_PLATFORM_ANDROID 1)
endif()

set(ENNOV_USE_LINUX_API_NATIVE BOOL CACHE 1 "Whether or not use native linux api")

if(ENNOV_USE_LINUX_AP_NATIVE)
  set(ENNOV_LINUX_API_SDL 1)
else()
  set(ENNOV_LINUX_API_X11 1)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(ENNOV_DEBUG 1)
endif()

# Initial Bulid options for Ennov
set(ENNOV_BUILD_OPTIONS)
set(ENNOV_LIB_TYPE)

if(ENNOV_DEBUG)
  set(ENNOV_BUILD_OPTIONS ${ENNOV_BUILD_OPTIONS} -DENNOV_DEBUG)
endif(ENNOV_DEBUG)

# All build options for linux
if(ENNOV_PLATFORM_LINUX)
  set(ENNOV_BUILD_OPTIONS ${ENNOV_BUILD_OPTIONS} -fno-rtti)
  set(ENNOV_LIB_TYPE STATIC)
endif()

# All build options for Android
if(ENNOV_PLATFORM_ANDROID)
  set(ENNOV_LIB_TYPE STATIC)
endif()

add_subdirectory(src/glm EXCLUDE_FROM_ALL)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

if(ENNOV_PLATFORM_LINUX)

  set(ENNOV_MAIN_SOURCE_FILE)

  if(ENNOV_LINUX_API_SDL)
    set(ENNOV_MAIN_SOURCE_FILE "src/sdl_ennov.cpp")
  elseif(ENNOV_LINUX_API_X11)
    set(ENNOV_MAIN_SOURCE_FILE "src/linux_ennov.cpp")
  endif()

  add_executable(linux_ennov ${ENNOV_MAIN_SOURCE_FILE})

  target_link_libraries(linux_ennov PUBLIC X11 GL GLX pthread dl glm::glm ennov m)
  target_include_directories(linux_ennov PUBLIC src/)
  target_compile_options(linux_ennov PUBLIC ${ENNOV_BUILD_OPTIONS})
  target_compile_features(linux_ennov PUBLIC c_std_99 cxx_std_11)
endif()

add_library(ennov ${ENNOV_LIB_TYPE} src/ennov.cpp)

target_link_libraries(ennov PUBLIC glm::glm)
target_compile_options(ennov PUBLIC ${ENNOV_BUILD_OPTIONS})
target_compile_features(ennov PUBLIC c_std_99 cxx_std_11)

# Include path to ennov_platform.h
target_include_directories(ennov PUBLIC src/)

if(ENNOV_PLATFORM_LINUX)
set_property(TARGET ennov PROPERTY PREFIX "")
endif()

# Add android cmake file
if(ENNOV_PLATFORM_ANDROID)
  add_subdirectory(android/app/src/main/cpp)
endif()
