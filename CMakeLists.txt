cmake_minimum_required(VERSION 3.20)
project(CESC450_RTOs_Baseline VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)

if(ENABLE_WARNINGS)
  if(MSVC)
    add_compile_options(/W4)
  else()
    add_compile_options(-Wall -Wextra -Wpedantic)
  endif()
endif()

# Define the rtos_shim library
add_library(rtos_shim
  rtos/rtos_shim.cpp
)
target_include_directories(rtos_shim PUBLIC include rtos)

# Define the main application and include all task files
add_executable(rtos_app
  src/main.cpp
  src/tasks/TaskA.cpp
  src/tasks/TaskB.cpp
)

# Link the shim library to your application
target_link_libraries(rtos_app PRIVATE rtos_shim)

# Helpful output
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
