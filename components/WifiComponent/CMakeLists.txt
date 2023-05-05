idf_component_register(SRCS "WifiComponent.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi nvs_flash log)
