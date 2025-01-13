#include "ESP32_VL53L0X.h" // Include the VL53L0X library
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define I2C_MASTER_SCL_IO 15      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 14      /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

VL53L0X sensor;

void i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0));
}

extern "C" void app_main()
{
    printf("Starting VL53L0X sensor example\n");

    // Initialize I2C
    i2c_master_init();

    // Initialize the VL53L0X sensor
    if (!sensor.begin())
    {
        printf("Failed to initialize VL53L0X\n");
        return;
    }

    printf("VL53L0X sensor initialized successfully\n");

    // Loop to read the distance and print it
    while (true)
    {
        // Read distance in millimeters
        uint16_t distance = sensor.readRangeSingleMillimeters();
        if (sensor.timeoutOccurred())
        {
            printf("Sensor timeout occurred!\n");
        }
        else
        {
            printf("Distance: %d mm\n", distance);
        }

        // Delay before the next reading
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
