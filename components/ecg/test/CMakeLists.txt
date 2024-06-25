idf_component_register(SRCS "testECGData.cpp" "MockECGDataProvider.cpp" 
                       INCLUDE_DIRS "."
                       REQUIRES unity ecg ecg_data_provider)