idf_component_register(SRCS "fullnessTask.cpp"
                            "usageTask.cpp"
                            "weightTask.cpp"
                            "clientTask.cpp"
                            "servoTask.cpp"
                            "cameraTask.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos FullnessComponent ClientComponent TaskComponent log driver nvs_flash
                        json spiffs esp_http_server esp_wifi esp_netif esp_timer
                    PRIV_REQUIRES nvs_flash esp_psram driver esp32-camera unity mbedtls
                    )
