idf_component_register(SRCS "usage_sensor.cpp" "fullness_sensor.cpp" "weight_sensor.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES log
                        PRIV_REQUIRES hx711 ultrasonic driver
                        )