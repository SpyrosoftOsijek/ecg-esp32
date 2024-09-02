idf_component_register(SRCS "bluetooth_ecg_wrapper.cpp"
                  INCLUDE_DIRS "include"
                  REQUIRES bt nvs_flash spi_flash)
