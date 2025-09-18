#include "initialization.hpp"
#include "esp_log.h"

static const char* TAG = "initialization";

void initialization(void) {
    sys_init_eg = xEventGroupCreate();
    ESP_LOGI(TAG, "System initialization event group created");
}