
#include <nvs.h>
#include <esp_err.h>
#include <cJSON.h>
#include <client_publish.hpp> 

/*
Assuming a JSON command is sent like this:
{
"location":""
"name":""
"ssid":""
"pswd":""

}
*/



esp_err_t setNVS(const char* value, const char* key){
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
need to figure out security issues
*/

esp_err_t parseCommand(const char* command){
    cJSON *json = cJSON_Parse(command);
    if(json == NULL){
        cJSON_Delete(json);
        sendSignal("Invalid Command")
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
    }ß
    cJSON_Delete(json);
    sendSignal("Succesfully Received Commandß")
    return ESP_OK;
}

/*
get value function if required
*/
esp_err_t get_value_nvs(const char* key,char* value,size_t size){
    nvs_handle_t handler;
    size_t size = 0;
    err = nvs_open("config", NVS_READWRITE, &handler)
    if(err!=ESP_OK){
        return err;
    }
    err = nvs_get_str(handler,key,value,&size);
    nvs_close(handler);
    return err; 
  }

  /*
  Function to send back a response to the server
  This is optional, but it could just be an error check or something like that
  */
  void sendSignal(const char* payload){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"response",*payload);
    char* payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    client_publish(payload);
  }

        



