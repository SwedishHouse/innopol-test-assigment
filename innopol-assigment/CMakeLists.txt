# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(innopol-assigment C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

# Найти все файлы .c и .cpp в директории src
file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")

# Add executable. Default name is the project name, version 0.1

add_executable(innopol-assigment innopol-assigment.c  ${PROJECT_SOURCES})

pico_set_program_name(innopol-assigment "innopol-assigment")
pico_set_program_version(innopol-assigment "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(innopol-assigment 1)
pico_enable_stdio_usb(innopol-assigment 0)

# Add the standard library to the build
target_link_libraries(innopol-assigment
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(innopol-assigment PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Add any user requested libraries
target_link_libraries(innopol-assigment 
        hardware_timer
        pico_cyw43_arch_none
        FreeRTOS-Kernel-Heap4
        )

pico_add_extra_outputs(innopol-assigment)

# Single core project 
add_compile_definitions(configNUMBER_OF_CORES=1)