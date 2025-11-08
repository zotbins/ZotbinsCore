idf_component_register(SRCS "servo.cpp" "camera_sensor.cpp" "usage_sensor.cpp" "fullness_sensor.cpp" "weight_sensor.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES log freertos mcp23x17
                        PRIV_REQUIRES hx711 ultrasonic driver peripheral_manager 
                        )