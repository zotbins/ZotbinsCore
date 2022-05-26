#include "RealWeight.hpp"
#include <Arduino.h>

using namespace Weight;


RealWeight::RealWeight()
 : mSensor{HX711{}}
{
}

void RealWeight::setup()
{
    mSensor.begin(hx_out_pin, hx_sck_pin);
    mSensor.set_scale(mCalibrationFactor);
    log_i("BEGIN!");
    /*
    while (!mSensor.is_ready())
    {
        log_i("not ready");
        vTaskDelay(1);
    }
    */
   while (digitalRead(hx_out_pin))
   {
       log_i("Pin is high");
        vTaskDelay(100);

   }
   /*
   while (digitalRead(hx_out_pin))
   {
       log_i("Pin is high");
       vTaskDelay(100);
   }
    */
    log_i("READY!");
    //mSensor.tare();
}
int32_t RealWeight::getWeight()
{
    // TODO !!!: ADD ERROR HANDLING
    log_i("Before power up");
    //mSensor.power_up();
    log_i("After");
    // MAYBE: add a delay... vTaskDelay(...)
    // requires testing...

    
    bool is_sensor_ready = mSensor.is_ready();
    
    if (!is_sensor_ready)
    {
        log_e("Not ready");
        return -1;
    }
    
    //int32_t reading = mSensor.get_units(10);
    //log_i("Read the result!");
    //mSensor.power_down();
    return 5; // returns the (avg of 10 raw readings from the ADC) - tare
                                  // all divided by our calibration factor
}