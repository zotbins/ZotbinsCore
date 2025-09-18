idf_component_register(SRCS "sensor_queue.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos json log
                    PRIV_REQUIRES publish peripheral_interface
                    )