# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.16)

# Include the components directory of the main application:
#
set(EXTRA_COMPONENT_DIRS "../components")

# Set the components to include the tests for.
# This can be overriden from CMake cache:
# - when invoking CMake directly: cmake -D TEST_COMPONENTS="xxxxx" ..
# - when using idf.py: idf.py -T xxxxx build
#
set(TEST_COMPONENTS "ecg" CACHE STRING "List of components to test")

add_compile_definitions(CONFIG_EMULATE_UART_GPIO_TX=16)
add_compile_definitions(CONFIG_EMULATE_UART_GPIO_RX=17)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ecg_test)