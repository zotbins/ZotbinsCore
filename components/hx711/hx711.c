/*
 * Copyright (c) 2019 Ruslan V. Uss <unclerus@gmail.com>
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
 * @file hx711.c
 *
 * ESP-IDF driver for HX711 24-bit ADC for weigh scales
 *
 * Copyright (c) 2019 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <esp_idf_lib_helpers.h>
#include "hx711.h"

#include "mcp23x17.h"

#define CHECK(x)                \
    do                          \
    {                           \
        esp_err_t __;           \
        if ((__ = x) != ESP_OK) \
            return __;          \
    } while (0)
#define CHECK_ARG(VAL)                  \
    do                                  \
    {                                   \
        if (!(VAL))                     \
            return ESP_ERR_INVALID_ARG; \
    } while (0)

#if HELPER_TARGET_IS_ESP32
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#endif

#ifndef BIT64
#define BIT64 BIT
#endif

// gpio_num_t dout, gpio_num_t pd_sck, hx711_gain_t gain)
static uint32_t read_raw(hx711_t *dev)
{
#if HELPER_TARGET_IS_ESP32
    portENTER_CRITICAL(&mux);
#elif HELPER_TARGET_IS_ESP8266
    portENTER_CRITICAL();
#endif

    // read data
    uint32_t data = 0;
    bool bit = 0;

    // 24 pulses to read data
    for (size_t i = 0; i < 24; i++)
    {
        mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, 1); // set clock high
        ets_delay_us(1);
        mcp_gpio_read(dev->mcp_dev_addr, dev->dout, &bit); // read bit
        data |= (bit << (23 - i));
        mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, 0); // set clock low
        ets_delay_us(1);

        /* gpio_set_level(pd_sck, 1);
        ets_delay_us(1);
        data |= gpio_get_level(dout) << (23 - i);
        gpio_set_level(pd_sck, 0);
        ets_delay_us(1); */
    }

    // config gain + channel for next read
    for (size_t i = 0; i <= dev->gain; i++)
    {
        mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, 1);
        ets_delay_us(1);
        mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, 0);
        ets_delay_us(1);

        /* gpio_set_level(pd_sck, 1);
        ets_delay_us(1);
        gpio_set_level(pd_sck, 0);
        ets_delay_us(1);
        */
    }

#if HELPER_TARGET_IS_ESP32
    portEXIT_CRITICAL(&mux);
#elif HELPER_TARGET_IS_ESP8266
    portEXIT_CRITICAL();
#endif

    return data;
}

///////////////////////////////////////////////////////////////////////////////

esp_err_t hx711_init(hx711_t *dev)
{
    CHECK_ARG(dev && dev->mcp_dev_addr);
    /*
    gpio_config_t conf = {
        .pin_bit_mask = BIT64(dev->dout),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    CHECK(gpio_config(&conf));

    conf.pin_bit_mask = BIT64(dev->pd_sck);
    conf.mode = GPIO_MODE_OUTPUT;
    CHECK(gpio_config(&conf));

    CHECK(hx711_power_down(dev, false));
    */

    CHECK(mcp_gpio_set_direction(dev->mcp_dev_addr, dev->dout, false));  // set DOUT as input
    CHECK(mcp_gpio_set_direction(dev->mcp_dev_addr, dev->pd_sck, true)); // set PD_SCK as output
    CHECK(mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, 0));            // set PD_SCK low

    // power up and set gain
    CHECK(hx711_power_down(dev, false));
    return hx711_set_gain(dev, dev->gain);
}

esp_err_t hx711_power_down(hx711_t *dev, bool down)
{
    CHECK_ARG(dev);
    CHECK(mcp_gpio_write(dev->mcp_dev_addr, dev->pd_sck, down ? 1 : 0)); // set PD_SCK high to power down, low to power up
    vTaskDelay(pdMS_TO_TICKS(1));                              // wait for power up/down

    return ESP_OK;
}

esp_err_t hx711_set_gain(hx711_t *dev, hx711_gain_t gain)
{
    CHECK_ARG(dev && gain <= HX711_GAIN_A_64);
    CHECK(hx711_wait(dev, 200)); // 200 ms timeout
    read_raw(dev);
    dev->gain = gain;
    return ESP_OK;
}

esp_err_t hx711_is_ready(hx711_t *dev, bool *ready)
{
    CHECK_ARG(dev && ready);
    bool dout_level;
    CHECK(mcp_gpio_read(dev->mcp_dev_addr, dev->dout, &dout_level));
    *ready = !dout_level;
    return ESP_OK;
}

esp_err_t hx711_wait(hx711_t *dev, size_t timeout_ms)
{
    bool ready = false;
    uint64_t started = esp_timer_get_time() / 1000;

    while (esp_timer_get_time() / 1000 - started < timeout_ms)
    {
        CHECK(hx711_is_ready(dev, &ready));
        if (ready)
            return ESP_OK;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return ESP_ERR_TIMEOUT;
}

esp_err_t hx711_read_data(hx711_t *dev, int32_t *data)
{
    CHECK_ARG(dev && data);
    // dev->dout, dev->pd_sck, dev->gain
    uint32_t raw = read_raw(dev);
    if (raw & 0x800000)
        raw |= 0xff000000;
    *data = *((int32_t *)&raw);

    return ESP_OK;
}

esp_err_t hx711_read_average(hx711_t *dev, size_t times, int32_t *data)
{
    CHECK_ARG(dev && times && data);

    int32_t v;
    *data = 0;
    for (size_t i = 0; i < times; i++)
    {
        CHECK(hx711_wait(dev, 200));
        CHECK(hx711_read_data(dev, &v));
        *data += v;
    }
    *data /= (int32_t)times;

    return ESP_OK;
}
