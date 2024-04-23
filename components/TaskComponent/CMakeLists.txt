idf_component_register(SRCS "fullnessTask.cpp"
                            "usageTask.cpp"
                            "weightTask.cpp"
                            "clientTask.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos FullnessComponent WeightComponent ClientComponent log )
