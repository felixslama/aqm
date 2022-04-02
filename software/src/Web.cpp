#include <Update.h>
#include "Pages.h"
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>

using namespace httpsserver;

#include "cert.h"
#include "private_key.h"

bool OTAinit = false;

SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);
HTTPSServer secureServer = HTTPSServer(&cert);
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleUpdate(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);
void handleDoUpdate(HTTPRequest * req, HTTPResponse * res);
void serverTask(void *params);
HTTPBodyParser *parser;
void initHTTPS(){
  Serial.println("init");
  xTaskCreatePinnedToCore(serverTask, "https443", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void serverTask(void *params){
  Serial.println("Creating the certificate was successful");
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeUpdate  = new ResourceNode("/update", "GET",&handleUpdate);
  ResourceNode * nodeUpdatePost = new ResourceNode("/update", "POST",&handleDoUpdate);
  ResourceNode * nodeUpdatePut = new ResourceNode("/update", "PUT",&handleDoUpdate);
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeUpdate);
  secureServer.registerNode(nodeUpdatePost);
  secureServer.registerNode(nodeUpdatePut);
  Serial.println("Starting server...");
  secureServer.start();
  Serial.println("woah after start");
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
    while(true){
      secureServer.loop();
    }
  }
}

bool checkOTAStatus() {
  return OTAinit;
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/html");
  res->println(aqmIndex);
}

void handleUpdate(HTTPRequest * req, HTTPResponse * res) {
  res->println(updateIndex);
}

void updateFirmware(uint8_t *data, size_t len){
  if (Update.write(data, len) == len) {
    int progress = Update.progress() / 10000;
    Serial.println(progress);
  } else {
    Update.printError(Serial);
  }
  return;
}

void handleDoUpdate(HTTPRequest * req, HTTPResponse * res) {
  OTAinit = true;
  Serial.println("OTA Initiated...");
  // jumpscare
  delay(2000);
  parser = new HTTPMultipartBodyParser(req);
  Update.begin(UPDATE_SIZE_UNKNOWN);
  parser->nextField();
  while (!parser->endOfField()) {
    byte buffer[512];
    size_t size = parser->read(buffer, 512);
    updateFirmware(buffer, size);
  }
  Update.end(true);
  ESP.restart();
}
