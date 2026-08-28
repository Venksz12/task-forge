cmake_minimum_required(VERSION 3.25)
project(TaskForge VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(TASKFORGE_BUILD_TESTS "Build unit tests" ON)

add_library(taskforge_common
  common/src/models.cpp
  common/src/thread_pool.cpp
  common/src/scheduler.cpp
  common/src/workflow.cpp
  common/src/retry.cpp
  common/src/metrics.cpp
  common/src/auth.cpp
  common/src/infrastructure.cpp)
target_include_directories(taskforge_common PUBLIC common/include)
target_compile_features(taskforge_common PUBLIC cxx_std_23)
if(MSVC)
  target_compile_options(taskforge_common PRIVATE /W4)
else()
  target_compile_options(taskforge_common PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(gateway)
add_subdirectory(services/scheduler)
add_subdirectory(services/worker)
if(TASKFORGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
