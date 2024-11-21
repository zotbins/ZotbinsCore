# target_compile_options(Client.cpp PRIVATE -fpermissive)

idf_component_register(SRCS "Client.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES log esp_system esp_partition nvs_flash esp_event esp_netif protocol_examples_common mqtt app_update)
