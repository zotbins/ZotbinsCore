#pragma once

#include <stdbool.h>
#include <stdint.h>

void sleep_manager_init(void);
void sleep_manager_task(void *arg);
void record_breakbeam_activity(void);
