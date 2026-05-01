idf_component_register( 
    SRCS 
        "sdcard.c"
        "camera_sd.c"
        
    INCLUDE_DIRS 
        "."

    REQUIRES 
        log 
        freertos 
        esp_netif 
        esp_event 
        lwip 
        driver        
        esp_wifi      
        esp_timer
        nvs_flash

        # --- NEW for camera + SD ---
        esp32-camera
        fatfs
        sdmmc
)