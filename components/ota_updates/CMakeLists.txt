idf_component_register(SRCS "advanced_https_ota_example.c" "ble_helper/bluedroid_gatts.c" "ble_helper/nimble_gatts.c"
                    "ble_helper/ble_api.c"
                    INCLUDE_DIRS "." "./ble_helper/include/"
                    REQUIRES 
                        esp_https_ota 
                        esp_event 
                        esp_netif 
                        esp_wifi 
                        app_update 
                        nvs_flash

                    # Embed the server root certificate into the final binary
                    EMBED_TXTFILES ./test_certs/server_cert.pem)# ${project_dir}/components/ota_updates/test_certs/server_cert.pem)
