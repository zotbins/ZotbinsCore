idf_component_register(
    SRCS "sleepManager.c" "power_sleep.c"
    INCLUDE_DIRS "."
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
)