idf_component_register(SRCS "serialize.cpp" "peripheral_queue.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos json log
                    PRIV_REQUIRES peripheral_interface
                    )