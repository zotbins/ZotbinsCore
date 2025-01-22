idf_component_register(SRCS "usageTask.cpp" "fullnessTask.cpp"
                           
                            "weightTask.cpp"
                            "clientTask.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos FullnessComponent WeightComponent ClientComponent TaskComponent log driver nvs_flash)
