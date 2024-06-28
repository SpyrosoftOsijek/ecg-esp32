idf_component_register(SRCS "test_ecg_data.cpp" "mock_ecg_data_provider.cpp" 
                       INCLUDE_DIRS "include"
                       REQUIRES unity ecg ecg_data_provider)