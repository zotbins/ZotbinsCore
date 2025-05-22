#include "servoTask.hpp"
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>

using namespace Zotbins;

// Please consult the datasheet of your servo before changing the following parameters
#define SERVO_MIN_PULSEWIDTH_US 1000  // Minimum pulse width in microsecond (original 1000)
#define SERVO_MAX_PULSEWIDTH_US 2000 // Maximum pulse width in microsecond (originally 2000)
#define SERVO_MIN_DEGREE -90         // Minimum angle
#define SERVO_MAX_DEGREE 90          // Maximum angle

#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000          // 20000 ticks, 20ms

// MAKE SURE SERVO GROUND PIN SHARES ESP32 GROUND PIN
const gpio_num_t PIN_SERVO = GPIO_NUM_15; // 16 IS NOT ANALOG SO DONT USE THAT FOR ESPCAM
// const gpio_num_t inputPIN = GPIO_NUM_32;

// TODO: get a direct access mapping to MCU's available GPIO pins or something instead
// const gpio_num_t PIN_SEND_MCU = GPIO_NUM_13;
// const gpio_num_t PIN_RECEIVE_MCU = GPIO_NUM_14;

// const gpio_config_t PIN_SEND_MCU_CONFIG = {
//     .pin_bit_mask = (1ULL << PIN_SEND_MCU),
//     .mode = GPIO_MODE_OUTPUT,
//     .pull_up_en = GPIO_PULLUP_DISABLE,
//     .pull_down_en = GPIO_PULLDOWN_DISABLE,
//     .intr_type = GPIO_INTR_DISABLE
// };

// const gpio_config_t PIN_RECEIVE_MCU_CONFIG = {
//     .pin_bit_mask = (1ULL << PIN_RECEIVE_MCU),
//     .mode = GPIO_MODE_INPUT,
//     .pull_up_en = GPIO_PULLUP_DISABLE,
//     .pull_down_en = GPIO_PULLDOWN_ENABLE, // Prevent floating
//     .intr_type = GPIO_INTR_DISABLE
// };

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

static const char *name = "servoTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static const int core = 1;

static TaskHandle_t xTaskToNotify = NULL;

// Servo Constants
int angle = 0;
int step = 1;
float angleDelay = 10;
int waitTime = 1000;
int targetAngle = 270;


ServoTask::ServoTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

// TODO: implement servo with task notifs
void ServoTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
    // xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
}

void ServoTask::taskFunction(void *task)
{
    ServoTask *servoTask = static_cast<ServoTask *>(task);
    servoTask->setup();
    servoTask->loop();
}

void ServoTask::setup()
{
    // ESP_ERROR_CHECK(gpio_config(&PIN_SEND_MCU_CONFIG)); // NECESSARY FOR SOME PINS!!
    // ESP_ERROR_CHECK(gpio_config(&PIN_RECEIVE_MCU_CONFIG));
}

mcpwm_cmpr_handle_t comparator = NULL;
mcpwm_gen_handle_t generator = NULL;
void ServoTask::loop()
{
    // FOR SERVO MAKE SURE ALL THE GROUNDS ARE COMMON AND SHARED OR ELSE THIS DOESNT WORK
    ESP_LOGI(name, "Create timer and operator");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(name, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    ESP_LOGI(name, "Create comparator and generator from the operator");
    //mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags = {
            .update_cmp_on_tez = true,
        }};
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    // mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = PIN_SERVO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    // ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));

    ESP_LOGI(name, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                            MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(name, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    while (1)
    {
        ESP_LOGI(name, "waiting for task response");
        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);

        mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(-10));
        for(int i = -10; i>= -180; i--){
            mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(i));
            vTaskDelay(3  / portTICK_PERIOD_MS);
        }
        xTaskToNotify = xTaskGetHandle("cameraTask"); 
        xTaskNotifyGive(xTaskToNotify); 
        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
        mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(-10));

        // set ultrasonic of SENSOR esp32 to low, telling it to activate
        ESP_LOGI("Servo", "starting servo thingy");
        gpio_set_level(GPIO_NUM_13, 1);
        gpio_set_level(GPIO_NUM_14, 0);

        // go back to usage 
        xTaskToNotify = xTaskGetHandle("usageTask");        
        // vTaskResume(xTaskToNotify);
        xTaskNotifyGive(xTaskToNotify); 
        vTaskDelay(100 / portTICK_PERIOD_MS);  
    }
    vTaskDelete(NULL);
}




void ServoTask::rotate() {
    step = 1;  // Reverse the direction
    while (angle <= targetAngle) {
        ESP_LOGI(name, "Angle of rotation: %d", angle);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        vTaskDelay(pdMS_TO_TICKS(angleDelay));
        angle += step;  // Move the servo in the negative direction (down)
    }

    step *= -1; 
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    while (angle >= 0) {
        ESP_LOGI(name, "Angle of rotation: %d", angle);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        vTaskDelay(pdMS_TO_TICKS(angleDelay));
        angle += step;  // Move the servo in the negative direction (down)
    }
}