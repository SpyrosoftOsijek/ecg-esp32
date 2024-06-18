idf_component_register(SRCS "UARTManager.cpp"
                  INCLUDE_DIRS "include"
                  REQUIRES  soft_uart ecg_data_provider)