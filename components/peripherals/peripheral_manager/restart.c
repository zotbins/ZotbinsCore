#include "esp_err.h"
#include "esp_system.h"
 

#define restartFlag (1<<0);

EventGroupHandle_t restartGroup;
void restartTask(){

}   
/*
add an ISR
ISR sets the bit
once the bit is check check if the restart flag is yes or no
if it is yes
call restart function and restart
*/

void updateRestart(void){
    BaseType_t HigherPriorityTaskWoken;
    HigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(
        restartGroup,
        restartFlag,
        &HigherPriorityTaskWoken
    );
    portYIELD_FROM_ISR(HigherPriorityTaskWoken);

}

esp_err_t restart(){    

}

