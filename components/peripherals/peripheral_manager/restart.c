#include "esp_err.h"
#include "esp_system.h"
#include "client_publish.hpp"

#define systemFlag (1<<0);
#define serverFlag (1<<0);
EventGroupHandle_t restartGroup = xEventGroupCreate();
void restartTask(EventGroupHandle_t restartGroup){
    for(;;){
    /*
    This Waits for the restart to be sent by the server
    */
    xEventGroupWaitBits(
        restartGroup,
        serverFlag,
        pdTrue,
        pdTrue,
        portMAX_DELAY       
    );
    /*This waits for the system to be available*/
    xEventGroupWaitBits(
        restartGroup,
        systemFlag,
        pdTrue,
        pdTrue,
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
    elif(requirement == 1){
    xEventGroupSetBits(
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