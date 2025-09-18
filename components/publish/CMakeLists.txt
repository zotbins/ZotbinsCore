idf_component_register(SRCS "client_publish.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt log freertos
                    PRIV_REQUIRES mqtt_client
                    )