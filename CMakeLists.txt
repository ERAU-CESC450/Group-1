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

# Define the fundamental RTOS shim environment
add_library(rtos_shim
  rtos/rtos_shim.cpp
 "src/ipc/ipc.h")
target_include_directories(rtos_shim PUBLIC include rtos)

# Build the final executable with both application tasks explicitly included
add_executable(rtos_app
   src/main.cpp
    src/tasks/tasks.cpp
    src/tasks/watchdog.cpp
    src/tasks/producer.cpp
    src/tasks/consumer.cpp
    src/tasks/log_worker.cpp
    src/timing/timing.cpp
    src/shared/heartbeat.cpp
    src/shared/safe_log.cpp
    src/ipc/ipc.cpp    

 "src/ipc/ipc.h")
 target_include_directories(rtos_app PRIVATE src/tasks src/timing src/shared src/ipc)
target_link_libraries(rtos_app PRIVATE rtos_shim)

# Diagnostics
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
