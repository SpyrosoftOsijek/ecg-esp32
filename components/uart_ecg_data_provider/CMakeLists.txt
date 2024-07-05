idf_component_register(SRCS "uart_ecg_data_provider.cpp"
                  INCLUDE_DIRS "include"
                  REQUIRES  soft_uart ecg_data_provider)
                  