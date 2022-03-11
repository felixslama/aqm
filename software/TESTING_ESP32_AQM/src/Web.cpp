#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "Pages.h"

AsyncWebServer server(80);

void handleUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/html", updateIndex);
};

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index){
    size_t content_len;  
    content_len = request->contentLength();
    int cmd = (filename.indexOf(F(".spiffs.bin")) > -1 ) ? U_SPIFFS : U_FLASH;
    if (cmd == U_FLASH && !(filename.indexOf(F(".bin")) > -1) ) return; // wrong image for ESP32
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) == len) {
    int progress = Update.progress() / 10000;
    Serial.println(progress);
  } else {
    Update.printError(Serial);
  }
  if (final) {    
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", updateDoneIndex); 
      request->send(response); 
      delay(100);
      ESP.restart();
    }
  }
}

void initWeb(){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", aqmIndex);
    });
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", updateIndex);
    });
    server.on("/doUpdate", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        handleDoUpdate(request, filename, index, data, len, final);}
    );
    server.begin();
    Serial.println("HTTP server started");
}