#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vl53l0x.h" // Include VL53L0X library
#include <stdio.h>

#define I2C_MASTER_SCL_IO 15      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 14      /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

#define XSHUT_PIN -1             /*!< Set to -1 if not using XSHUT */
#define VL53L0X_I2C_ADDRESS 0x29 /*!< Default I2C address for VL53L0X */
#define IO_2V8 0                 /*!< Set to 1 if using 2.8V logic, 0 if using 3.3V/5V */

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

extern "C" void app_main(void)
{
    // Initialize I2C
    i2c_master_init();

    vl53l0x_t *sensor = vl53l0x_config(I2C_MASTER_NUM, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO, XSHUT_PIN, VL53L0X_I2C_ADDRESS, IO_2V8);

    printf("Starting VL53L0X sensor example\n");

    // Initialize the VL53L0X sensor
    if (vl53l0x_init(sensor) == NULL)
    {
        printf("VL53L0X sensor initialized successfully\n");
    }
    else
    {
        printf("Failed to initialize VL53L0X\n");
    }

    return;

    // Loop to read the distance and print it
}
