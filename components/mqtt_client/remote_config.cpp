
#include <client_publish.hpp> 
#include "remote_config.hpp" 
#include <nvs.h>
#include <esp_err.h>
#include <cJSON.h>
#include <string.h> 
/*
Assuming a JSON command is sent like this:
{
"location":""
"name":""
"ssid":""
"pswd":""
}
*/



void sendSignal(const char* payload);

esp_err_t setNVS(const char* value, const char* key){
    nvs_handle_t handler;
    esp_err_t err;
    err = nvs_open("config", NVS_READWRITE, &handler);
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
need to figure out security issues
*/

void parseCommand(const char* command){
    cJSON *json = cJSON_Parse(command);
    if(json == NULL){
        cJSON_Delete(json);
        sendSignal("Invalid Command");
        return;
    }
    cJSON *location = cJSON_GetObjectItemCaseSensitive(json,"location");
    cJSON *name = cJSON_GetObjectItemCaseSensitive(json,"name");
    cJSON *ssid = cJSON_GetObjectItemCaseSensitive(json,"ssid");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(json,"password");

    if(cJSON_IsString(location) && location->valuestring!=NULL && strlen(location->valuestring)>0){
        setNVS("location",location->valuestring);
    }
    if(cJSON_IsString(name) && name->valuestring!=NULL && strlen(name->valuestring)>0){
        setNVS("name",name->valuestring);
    }
    if(cJSON_IsString(ssid) && ssid->valuestring!=NULL && strlen(ssid->valuestring)>0){
        setNVS("ssid",ssid->valuestring);
    }
    if(cJSON_IsString(password) && password->valuestring!=NULL && strlen(password->valuestring)>0){
        setNVS("password",location->valuestring);
    }
    cJSON_Delete(json);
    sendSignal("Succesfully Received Command");
}

/*
get value function if required
*/
/*
esp_err_t get_value_nvs(const char* key,char* value,size_t* size){
    
    nvs_handle_t handler;
    size_t size = 0;
    err = nvs_open("config", NVS_READWRITE, &handler);
    if(err!=ESP_OK){
        return err;
    }
    err = nvs_get_str(handler,key,value,&size);
    nvs_close(handler);
    return err; 
  }
*/


  /*
  Function to send back a response to the server
  This is optional, but it could just be an error check or something like that
  */
  void sendSignal(const char* payload){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"response",payload);
    char* new_payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    client_publish(new_payload);
  }