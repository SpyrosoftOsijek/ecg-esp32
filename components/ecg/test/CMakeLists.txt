idf_component_register(SRCS "testECGData.cpp" "MockECGDataProvider.cpp" 
                       INCLUDE_DIRS "include"
                       REQUIRES unity ecg ecg_data_provider)