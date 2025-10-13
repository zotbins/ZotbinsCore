#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "freertos/event_groups.h"
#include "peripheral_manager.hpp"
#include "events.hpp"

extern EventGroupHandle_t manager_eg; // Event group to signal when sensors have finished collecting data

#endif // EVENTS_HPP