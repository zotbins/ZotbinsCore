/*
 * Copyright (c) 2016 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file ultrasonic.c
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and the like
 *
 * Ported from esp-open-rtos
 *
 * Copyright (c) 2016 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#include <esp_idf_lib_helpers.h>
#include "ultrasonic.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <esp_intr_alloc.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include "esp_log.h"

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP_M 5800.0f
#define ROUNDTRIP_CM 58
#define HALF_SPEED_OF_SOUND_AT_0C_M_S 165.7 // Half speed of sound in m/s at 0 degrees Celsius
#define ECHO_BIT (1 << 0)

#if HELPER_TARGET_IS_ESP32
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&mux)

#elif HELPER_TARGET_IS_ESP8266
#define PORT_ENTER_CRITICAL portENTER_CRITICAL()
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL()

#else
#error cannot identify the target
#endif

#define timeout_expired(start, len) ((esp_timer_get_time() - (start)) >= (len))

#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)
#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define RETURN_CRITICAL(RES) do { PORT_EXIT_CRITICAL; return RES; } while(0)
EventGroupHandle_t xEventCheck; 
QueueHandle_t xTimes;

const char *TAG = "ultrasonic";

void IRAM_ATTR echo_isr_handler(void* parameters){
    BaseType_t xResult,xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    
    int64_t time;

    // Sets ECHO_BIT and sends time through queue
    time = esp_timer_get_time();
    xQueueSendFromISR(xTimes,&time,&xHigherPriorityTaskWoken);

    xResult = xEventGroupSetBitsFromISR(xEventCheck,ECHO_BIT,&xHigherPriorityTaskWoken);

    if(xHigherPriorityTaskWoken == pdTRUE){     
        portYIELD_FROM_ISR();
    }
}

esp_err_t ultrasonic_init(const ultrasonic_sensor_t *dev)
{
    CHECK_ARG(dev);
    CHECK(gpio_set_direction(dev->trigger_pin, GPIO_MODE_OUTPUT));
    CHECK(gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT));
    xEventCheck = xEventGroupCreate();

    xTimes = xQueueCreate(
        2,
        sizeof(int64_t)
    );
    // Sets ISR to active on rising edge and falling edge
    ESP_LOGI(TAG, "Setting up ISR handler for echo pin...");
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_intr_type(dev->echo_pin, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_isr_handler_add(dev->echo_pin,echo_isr_handler,NULL));
    ESP_LOGI(TAG, "ISR handler configured!");
    return gpio_set_level(dev->trigger_pin, 0);
}


esp_err_t ultrasonic_measure_raw(const ultrasonic_sensor_t *dev, uint32_t max_time_us, uint32_t *time_us)
{

    xQueueReset(xTimes);  // Clear any stale data
    xEventGroupClearBits(xEventCheck, ECHO_BIT);

    CHECK_ARG(dev && time_us);
    EventBits_t timeBits;
    TickType_t startTimeout = pdMS_TO_TICKS(PING_TIMEOUT);
    int64_t start;
    int64_t time;

    // Ping: Low for 2..4 us, then high 10 us
    ESP_LOGI(TAG, "Sending ping...");
    CHECK(gpio_set_level(dev->trigger_pin,0));
    ets_delay_us(TRIGGER_LOW_DELAY);
    CHECK(gpio_set_level(dev->trigger_pin,1));
    ets_delay_us(TRIGGER_HIGH_DELAY);
    CHECK(gpio_set_level(dev->trigger_pin,0));

    // Previous ping isn't ended
    if (gpio_get_level(dev->echo_pin))
        RETURN_CRITICAL(ESP_ERR_ULTRASONIC_PING);

    //Waits for Start Bit
    ESP_LOGI(TAG, "Waiting for echo rising edge...");
    timeBits = xEventGroupWaitBits(
        xEventCheck,
        ECHO_BIT,
        pdTRUE,
        pdTRUE,
        startTimeout    
    );
    ESP_LOGI(TAG, "Echo rising edge received!");
    
    // Returns Error if no value is passed by Queue
    if(xQueueReceive(xTimes,&start,0)!=pdTRUE){
        return ESP_ERR_ULTRASONIC_ECHO_TIMEOUT;
    }

    //Waits for Stop Bit
    ESP_LOGI(TAG, "Waiting for echo falling edge...");
    timeBits = xEventGroupWaitBits(
        xEventCheck,
        ECHO_BIT,
        pdTRUE,
        pdTRUE,
        startTimeout
    ); 
    ESP_LOGI(TAG, "Echo falling edge received!");

    // Returns Error if no value is passed by Queue
    if(xQueueReceive(xTimes,&time,0)!=pdTRUE){
        return ESP_ERR_ULTRASONIC_ECHO_TIMEOUT;
    }

    // Calculate time of pulse
    *time_us = time - start;
    xQueueReset(xTimes);
    return ESP_OK;
}

esp_err_t ultrasonic_measure(const ultrasonic_sensor_t *dev, float max_distance, float *distance)
{
    CHECK_ARG(dev && distance);

    uint32_t time_us;
    CHECK(ultrasonic_measure_raw(dev, max_distance * ROUNDTRIP_M, &time_us));
    *distance = time_us / ROUNDTRIP_M;

    return ESP_OK;
}

esp_err_t ultrasonic_measure_cm(const ultrasonic_sensor_t *dev, uint32_t max_distance, uint32_t *distance)
{
    CHECK_ARG(dev && distance);

    uint32_t time_us;
    CHECK(ultrasonic_measure_raw(dev, max_distance * ROUNDTRIP_CM, &time_us));
    *distance = time_us / ROUNDTRIP_CM;

    return ESP_OK;
}

esp_err_t ultrasonic_measure_temp_compensated(const ultrasonic_sensor_t *dev, float max_distance, float *distance, float temperature_c)
{
    CHECK_ARG(dev && distance);

    // Calculate half (because of roundtrip) speed of sound in m/us based on temperature
    float speed_of_sound = (HALF_SPEED_OF_SOUND_AT_0C_M_S + 0.6 * temperature_c) / 1000000; // Convert m/s to m/us

    uint32_t time_us;
    // Adjust max_time_us based on the recalculated speed of sound
    CHECK(ultrasonic_measure_raw(dev, max_distance / speed_of_sound, &time_us));
    // Calculate distance using the temperature-compensated speed of sound
    *distance = time_us * speed_of_sound;

    return ESP_OK;
}

esp_err_t ultrasonic_measure_cm_temp_compensated(const ultrasonic_sensor_t *dev, uint32_t max_distance, uint32_t *distance, float temperature_c)
{
    CHECK_ARG(dev && distance);

    // Calculate half (because of roundtrip) speed of sound in cm/us based on temperature
    float speed_of_sound_cm_us = ((HALF_SPEED_OF_SOUND_AT_0C_M_S + 0.6 * temperature_c) * 100) / 1000000; // Convert m/s to cm/us

    uint32_t time_us;
    // Adjust max_time_us based on the recalculated speed of sound in cm
    CHECK(ultrasonic_measure_raw(dev, max_distance * 100 / speed_of_sound_cm_us, &time_us));
    // Calculate distance using the temperature-compensated speed of sound, converting result to cm
    *distance = time_us * speed_of_sound_cm_us;

    return ESP_OK;
}
