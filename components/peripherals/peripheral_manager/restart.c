#include "freertos/FreeRTOS.h" 
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_system.h"
#include "client_publish.hpp"
#include "restart.hpp"


#define systemFlag (1<<0)
#define serverFlag (1<<0)
EventGroupHandle_t restartGroup;
void restartTask(EventGroupHandle_t restartGroup){
    restartGroup = xEventGroupCreate();
    for(;;){
    /*
    This Waits for the restart to be sent by the server
    */
    xEventGroupWaitBits(
        restartGroup,
        serverFlag,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY       
    );
    /*This waits for the system to be available*/
    xEventGroupWaitBits(
        restartGroup,
        systemFlag,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY
    );
    restart();
    }
}   

void updateRestart(int requirement,EventGroupHandle_t restartGroup){    
    if(requirement == 0){
    xEventGroupSetBits(
        restartGroup,
        systemFlag
    );
    }
    else if(requirement == 1){
    xEventGroupClearBits(
        restartGroup,
        systemFlag
    );
    }
}

void restart(){
    /*Send message to Server and Restart*/
    client_publish("System is Restarting...");
    esp_restart();
}

void receiveServer(EventGroupHandle_t restartGroup){
    xEventGroupSetBits(
        restartGroup,
        serverFlag
    );
}   