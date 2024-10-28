idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES nvs_flash esp_psram driver esp32-camera unity mbedtls
                    REQUIRES json spiffs esp_http_server esp_wifi esp_netif esp_timer sdmmc
                    )
                    