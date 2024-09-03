idf_component_register(SRCS "ecg.cpp" 
                  INCLUDE_DIRS "include"
                  REQUIRES  soft_uart ecg_data_provider)