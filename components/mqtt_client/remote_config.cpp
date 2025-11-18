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
    return ESP_OK;
}

/*
Bare Bones of the Parsing Command
not the most scalable thing in the world
*/

esp_err_t parseCommand(const char* command){
    cJSON *json = cJSON_Parse(command);
    if(json == NULL){
        cJSON_Delete(json);
        return;
    }
    cJSON *location = cJSON_GetObjectItemCaseSensitive(json,"location");
    cJSON *name = cJSON_GetObjectItemCaseSensitive(json,"name");
    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(json,"ssid");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(json,"password");
    
    if(cJSON_IsString(location) && location->valuestring!=NULL && location->valuestring!=""){
        set_value("location",location->valuestring);
    }
    if(cJSON_IsString(name) && name->valuestring!=NULL && name->valuestring!=""){
        set_value("name",location->valuestring);
    }
    if(cJSON_IsString(ssid) && ssid->valuestring!=NULL && ssid->valuestring!=""){
        set_value("ssid",ssid->valuestring);
    }
    if(cJSON_IsString(password) && password->valuestring!=NULL && password->valuestring!=""){
        set_value("password",location->valuestring);
    }
    cJSON_Delete(json);
    return ESP_OK;
}






