idf_component_register( 
                        SRCS "DistanceBuffer.cpp" "FullnessMetric.cpp" "Distance.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES driver esp_timer freertos log ultrasonic
                      )
