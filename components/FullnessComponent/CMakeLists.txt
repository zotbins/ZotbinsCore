idf_component_register( 
                        SRCS "DistanceBuffer.cpp" "FullnessMetric.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES driver esp_timer freertos log ultrasonic
                      )
