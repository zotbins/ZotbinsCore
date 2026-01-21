#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "client_publish.hpp"

void sendMessage();
esp_err_t startTimer(){
    esp_timer_handle_t timer;
    const esp_timer_create_args_t timer_arguments ={
        .callback = &sendMessage,
        .arg = NULL,
        .name = "Monitors Zotbin's Bin Activity"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_arguments,&timer));
    /*
    Starts the timer, 
    Will callback function every 5 minutes (5 minutes -> 300000000 microseconds)
    */
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer,300000000));
    return ESP_OK;
}

void sendMessage(){
    client_publish("Bin is Alive");
}