idf_component_register( 
    SRCS 
        "app_main.cpp"
        "sleepManager.c" 
        "power_sleep.c"
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
        esp_camera
        esp_vfs_fat
        sdmmc
)