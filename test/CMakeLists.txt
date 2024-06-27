cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "../components")

set(TEST_COMPONENTS "ecg" "uart_ecg_data_provider" CACHE STRING "List of components to test")

add_compile_definitions(CONFIG_EMULATE_UART_GPIO_TX=16)
add_compile_definitions(CONFIG_EMULATE_UART_GPIO_RX=17)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_ecg_test)