#include "initialization.hpp"

extern EventGroupHandle_t sys_init_eg;

EventGroupHandle_t sys_init_eg = nullptr;

void initialize(void) {
    sys_init_eg = xEventGroupCreate();
}