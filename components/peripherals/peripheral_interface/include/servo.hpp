#ifndef SERVO_HPP
#define SERVO_HPP

#define SERVO_STATE_EVENT_BIT BIT1

#define GATE_OPEN 1
#define GATE_CLOSED 0

#include "esp_err.h"

/**
 * @file servo.hpp
 * @brief Servo control API for ESP32.
 *
 * Provides initialization and simple position control for a TowerPro
 * MG996R servo using PWM. All functions return ESP-IDF style error codes.
 */

/**
 * @brief Initialize the servo driver.
 *
 * Sets up the PWM timer/channel and any required GPIO for the servo signal.
 *
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_STATE if already initialized
 * - ESP_FAIL (or another esp_err_t) on hardware/config error
 */
esp_err_t init_servo(void);

/**
 * @brief Set the servo angle.
 *
 * Converts degrees to the proper PWM pulse and updates the servo output.
 *
 * @param degrees Target angle in degrees (e.g., 0–180). Values outside the
 *        supported range will be clamped or return an error depending on the
 *        implementation.
 *
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if degrees is out of range
 * - ESP_ERR_INVALID_STATE if the driver is not initialized
 */
esp_err_t servo_set_angle(float degrees);

/**
 * @brief Open servo preset - sets servo to 90 degrees.
 * 
 * @return esp_err_t 
 */
esp_err_t open_servo(void);

/**
 * @brief Close servo preset - sets servo to 0 degrees.
 * 
 * @return esp_err_t 
 */
esp_err_t close_servo(void);

/**
 * @brief Set the raw PWM pulse width sent to the servo.
 *
 * @param pulse_us Pulse width in microseconds (typical ~500–2500 µs).
 *
 * @return
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if pulse_us is out of range
 * - ESP_ERR_INVALID_STATE if the driver is not initialized
 */
esp_err_t servo_set_pulse_us(unsigned int pulse_us);

#endif // SERVO_HPP