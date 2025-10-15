idf_component_register(SRCS "gps_sensor.cpp" "servo.cpp" "camera_sensor.cpp" "usage_sensor.cpp" "fullness_sensor.cpp" "weight_sensor.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES log freertos
                        PRIV_REQUIRES hx711 ultrasonic driver peripheral_manager
                        )