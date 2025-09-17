idf_component_register(SRCS "client_publish.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt log esp_system esp_event
                    PRIV_REQUIRES mqtt_client
                    )