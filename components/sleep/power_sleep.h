#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void power_sleep_set_timezone_pacific(void);
bool power_sleep_sync_time_if_needed(void);
bool power_sleep_maybe_enter_night_deep_sleep(void);
void power_sleep_stop_radios(void);

#ifdef __cplusplus
}
#endif