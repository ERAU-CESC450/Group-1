cmake_minimum_required(VERSION 3.20)
project(CESC450_Module8_Stabilized VERSION 0.2.0 LANGUAGES C CXX)

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

add_library(rtos_shim
    rtos/rtos_shim.cpp
)

target_include_directories(rtos_shim PUBLIC
    include
    rtos
)

add_executable(rtos_app
    src/main.cpp
    src/interrupts/isr.cpp
    src/tasks/watchdog.cpp
    src/tasks/producer.cpp
    src/tasks/consumer.cpp
    src/tasks/log_worker.cpp
    src/timing/timing.cpp
    src/shared/heartbeat.cpp
    src/shared/safe_log.cpp
    src/shared/system_config.cpp
    src/shared/system_stats.cpp
    src/ipc/ipc.cpp
)

target_include_directories(rtos_app PRIVATE
    src/tasks
    src/timing
    src/shared
    src/ipc
    src/interrupts
)

target_link_libraries(rtos_app PRIVATE rtos_shim)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
