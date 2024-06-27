idf_component_register(SRCS "test_uart_ecg_data_provider.cpp"
                       REQUIRES unity uart_ecg_data_provider)

add_compile_definitions(UNIT_TESTING)
