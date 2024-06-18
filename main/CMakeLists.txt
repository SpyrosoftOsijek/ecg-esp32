set(srcs "soft_uart_main.cpp")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES driver soft_uart ecg  service_provider)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)

