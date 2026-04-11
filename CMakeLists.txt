cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Gallery)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Collect LVGL sources recursively
file(GLOB_RECURSE LVGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/LVGL/lvgl/src/*.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Drivers/LCD/lcd.c
    Drivers/BMP/bmp.c
    Drivers/SOFT_I2C/soft_i2c.c
    Drivers/TOUCH/gt911.c
    Drivers/LV_PORT_DISP/lv_port_disp.c
    Drivers/LV_PORT_INDEV/lv_port_indev.c
    Drivers/UI/ui_main.c
    Drivers/UI/photo_view.c
    ${LVGL_SOURCES}
    Drivers/JPEG/jpeg_viewer.c
    Middlewares/TJpgDec/tjpgd.c
    Drivers/IMAGE/image_scale.c
    Drivers/APP/album/album.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Drivers/LCD
    Drivers/BMP
    Drivers/SOFT_I2C
    Drivers/TOUCH
    Middlewares/LVGL/lvgl
    Middlewares/LVGL
    Drivers/LV_PORT_DISP
    Drivers/LV_PORT_INDEV
    Drivers/UI
    Drivers/JPEG
    Middlewares/TJpgDec
    Drivers/IMAGE
    Drivers/APP/album
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
