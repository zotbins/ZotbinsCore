idf_component_register(SRCS "fullnessTask.cpp"
                            "usageTask.cpp"
                            "weightTask.cpp"
                            "clientTask.cpp"
                            "servoTask.cpp"
                    INCLUDE_DIRS "include"
<<<<<<< Updated upstream
                    REQUIRES freertos FullnessComponent WeightComponent ClientComponent TaskComponent log driver)
=======
                    REQUIRES freertos FullnessComponent WeightComponent ClientComponent TaskComponent log driver nvs_flash )
>>>>>>> Stashed changes
