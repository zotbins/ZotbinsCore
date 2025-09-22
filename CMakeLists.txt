# The following four lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Removes user-specific path from dependencies.lock
set(EXTRA_COMPONENT_DIRS
    $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
    ${CMAKE_SOURCE_DIR}/components/peripherals
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(newZotbinsCore)

### DEFINE SPECIFIC HARDWARE -- THIS IS BEFORE PROJECT OVERHAUL (9/21/2025) ###

# add_compile_definitions(ESP32_CAM)
add_compile_definitions(ESP_WROVER)