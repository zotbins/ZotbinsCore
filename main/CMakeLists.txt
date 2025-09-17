idf_component_register(SRCS "app_main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt_client mqtt esp_system esp_event esp_netif nvs_flash protocol_examples_common
                    )
