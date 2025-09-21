#include "initialization.hpp"

EventGroupHandle_t sys_init_eg = nullptr;

void initialize(void)
{
    sys_init_eg = xEventGroupCreate();
}