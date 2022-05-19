#include "RealWeight.hpp"
#include <Arduino.h>

using namespace Weight;

RealWeight::RealWeight()
 : mSensor{HX711{}}
{
    mSensor.begin(hx_out_pin, hx_sck_pin);
    mSensor.set_scale(mCalibrationFactor);
    mSensor.tare();
}

int32_t RealWeight::getWeight()
{
    // TODO !!!: ADD ERROR HANDLING
    mSensor.power_up();
    // MAYBE: add a delay... vTaskDelay(...)
    // requires testing...

    bool is_sensor_ready = mSensor.wait_ready_timeout(2000, 10);

    if (!is_sensor_ready)
    {
        log_e("Sensor timed out :(");
        return -1;
    }

    int32_t reading = mSensor.get_units(10);
    mSensor.power_down();
    return reading; // returns the (avg of 10 raw readings from the ADC) - tare
                                  // all divided by our calibration factor
}