#include <esp_idf_lib_helpers.h>
#include <nvs.h>
#include <esp_err.h>
#include <cJSON.h>

/*Need to figure out what is the topic to be subscribed to and everything 
  Also need to figure out security 
  Also would json be the best addition since I would need a library to parse that data?
*/

/*
Assuming a JSON command is sent like this:
{
"location":""
"name":""
"ssid":""
"pswd":""

}
*/

/*
for now I would probably work on the NVS stuff 
*/




/*
Probably need some sort of configuration at the start?
*/

typdef struct configuration{
    const char* location,
    const char* name,
    const char* ssid,
    const char* password
}configuration;



esp_err_t set_value(const char* value, const char* key,){
    nvs_handle_t handler;
    esp_err_t err;
    err = nvs_open("config", NVS_READWRITE, &handler)
    if(err!=ESP_OK){
        return err;
    }
    err = nvs_set_str(handler,key,value);
    if(err!=ESP_OK){
        return err;
    }
    err = nvs_commit(handler);
    if(err!=ESP_OK){
        return err;
    }
    nvs_close(handler);
}







