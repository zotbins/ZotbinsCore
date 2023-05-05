#include <stdio.h>
#include <iostream>
#include "MockDistance.hpp"
#include "MockWeight.hpp"
#include "RealWeight.hpp"
#include "weightTask.hpp"
#include "message.hpp" 
#include "WifiComponent.hpp"
#include "esp_mac.h"

void print_mac_addr() {
	const char* TAG = "Extract MAC address";
	esp_err_t ret = ESP_OK;
	uint8_t base_mac_addr[6];
	ret = esp_efuse_mac_get_default(base_mac_addr);
	if(ret != ESP_OK){
			ESP_LOGE(TAG, "Failed to get base MAC address from EFUSE BLK0. (%s)", esp_err_to_name(ret));
			ESP_LOGE(TAG, "Aborting");
			abort();
		} 

	uint8_t index = 0;
	char macId[50];
	for(uint8_t i=0; i<6; i++){
	index += sprintf(&macId[index], "%02x", base_mac_addr[i]);
			}
	ESP_LOGI(TAG, "macId = %s", macId);

}

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{
	/*
	Fullness::MockDistance md{std::vector<int32_t>{1, 2}};
	md.getDistance();
	Weight::MockWeight mw{10};
	mw.getWeight();
	Weight::RealWeight rw;
	rw.getWeight();
	

	// START WIFI INITIALIZATION
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("WIFI", "ESP_WIFI_MODE_STA");
    wifi_init_sta();
	// END WIFI INITIALIZATION
	*/
    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    Zotbins::WeightTask weightTask(messageQueue);
	weightTask.start();
	
}
