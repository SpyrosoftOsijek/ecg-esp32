idf_component_register(SRCS "ECG.cpp" 
                  INCLUDE_DIRS "include"
                  REQUIRES  soft_uart uart_ecg_data_provider)

