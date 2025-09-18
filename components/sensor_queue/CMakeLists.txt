idf_component_register(SRCS "usage_sensor.cpp" "fullness_sensor.cpp" "weight_sensor.cpp" "sensor_queue.cpp" ""
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES publish sensor freertos json
                    )