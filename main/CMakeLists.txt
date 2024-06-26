set(srcs "main.cpp")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "."
                       REQUIRES  ecg uart_ecg_data_provider )
