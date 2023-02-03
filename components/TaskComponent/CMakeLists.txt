idf_component_register(SRCS "fullnessTask.cpp"
                            "usageTask.cpp"
                            "weightTask.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos FullnessComponent WeightComponent log)
