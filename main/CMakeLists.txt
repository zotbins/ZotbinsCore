idf_component_register(SRCS "app_main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash esp_wifi esp_event esp_system protocol_examples_common
                    PRIV_REQUIRES mqtt_client publish
                    )