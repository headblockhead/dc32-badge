cmake_minimum_required(VERSION 3.12)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(external/pico-sdk/pico_sdk_init.cmake)

project(slab C CXX ASM) # Name of project, can have multiple targets.
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Create files for CCLS to use for code completion.
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Require Pico SDK version 1.3.0
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
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
add_executable(slab
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        # The SSD1306 library is a weird one. It's not a library with a cmake file, so we have to add the source files manually.
        ${CMAKE_CURRENT_LIST_DIR}/external/pico-ssd1306/ssd1306.c
        )

# Add USB configuration files
target_sources(slab PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/src/keymap.c
        )

target_include_directories(slab PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
                )

# Generate the PIO header files
pico_generate_pio_header(slab ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)
pico_generate_pio_header(slab ${CMAKE_CURRENT_LIST_DIR}/src/quadrature_encoder.pio)

include_directories(${CMAKE_CURRENT_LIST_DIR}/external/squirrel/include) # Include SQUIRREL headers in target and dependencies.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/squirrel) # Add SQUIRREL as a subdirectory. This will build the library.

include_directories(${CMAKE_CURRENT_LIST_DIR}/external/pico_pca9555/include) # Include PCA9555 headers in target and dependencies.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/pico_pca9555) # Add PCA9555 as a subdirectory. This will build the library.

include_directories(${CMAKE_CURRENT_LIST_DIR}/external/pico-ssd1306) # Include SSD1306 headers in target and dependencies.

# Pull in common dependencies
target_link_libraries(slab pico_stdlib pico_unique_id tinyusb_device tinyusb_board pico_multicore hardware_gpio hardware_i2c squirrel pico_pca9555 hardware_pio)

# Enable UART output
pico_enable_stdio_uart(slab 1)

# Create map/bin/hex file etc.
pico_add_extra_outputs(slab)
