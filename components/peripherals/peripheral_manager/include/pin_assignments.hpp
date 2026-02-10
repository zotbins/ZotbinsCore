/* Up to date for WROVER-DEV_01_02 */

/* Pin assignments on MCP23017 */
const uint8_t PIN_TRIGGER = 2;                // GPIO pin for HC-SR04 trigger
const uint8_t PIN_ECHO = 8;                   // GPIO pin for HC-SR04 echo
const uint8_t PIN_CLOCK = 3;                  // GPIO pin for HX711 clock
const uint8_t PIN_DATA = 4;                   // GPIO pin for HX711 data
const uint8_t PIN_BREAKBEAM = 9;              // GPIO pin for breakbeam sensor

/* Pin assignments on WROVER */
const gpio_num_t PIN_SDA = GPIO_NUM_13;       // GPIO pin for I2C SDA
const gpio_num_t PIN_SCL = GPIO_NUM_14;       // GPIO pin for I2C SCL
const gpio_num_t PIN_INTERRUPT = GPIO_NUM_15; // GPIO pin for mcp23017 interrupt