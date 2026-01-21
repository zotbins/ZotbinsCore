#include <esp_timer.h>

/*
Implement an ISR routine that calls an eventbit and if that eventbit is activated call sendMessage?
Create an event bit 
and then the ISR routine sets the bit 
send message waits for that bit and once that bit is set it calls the send value function
*/


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
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer,300000000))
}

void sendMessage(){
    
}