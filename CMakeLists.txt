cmake_minimum_required(VERSION 3.12)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(external/pico-sdk/pico_sdk_init.cmake)

project(dc32_badge C CXX ASM) # Name of project, can have multiple targets.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Create files for CCLS to use for code completion.
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Require Pico SDK version 2.0.0 - uses the rp2350 chip.
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

# Add compiler flags:
add_compile_options(-Wall # Enable all warnings
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-Og) # Enable debug optimizations
else()
        add_compile_options(-O3) # Enable speed optimizations
endif()

# Target: Create an executable
add_executable(dc32_badge
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/HSV2RGB.c
        )

target_include_directories(dc32_badge PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
                )

# Generate the PIO header files
pico_generate_pio_header(dc32_badge ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)


#add_subdirectory(external/pico-extras)
#add_subdirectory(external/pico-displayDrivs/st7789)
#add_subdirectory(external/pico-displayDrivs/gfx)

# Pull in common dependencies
target_link_libraries(
        dc32_badge
        pico_stdlib
        #pico_unique_id
        pico_multicore
        hardware_gpio
        hardware_pio
        #pico_flash
        #hardware_exception
        #hardware_flash
        #st7789
        #gfx
        #hardware_pwm
        )

# Create map/bin/hex file etc.
pico_add_extra_outputs(dc32_badge)
