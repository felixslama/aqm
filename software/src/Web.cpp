#include <Update.h>
#include <functional>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include "Web.h"
#include "Pages.h"
#include "Cert.h"

using namespace httpsserver;

SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

HTTPSServer secureServer = HTTPSServer(&cert);
HTTPBodyParser *parser;

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define HEADER_USERNAME "X-USERNAME"
#define HEADER_GROUP    "X-GROUP"

bool OTAinit = false;
const char* WEBUSER;
const char* WEBPASS;

void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleUpdate(HTTPRequest * req, HTTPResponse * res);
void handleDoUpdate(HTTPRequest * req, HTTPResponse * res);
void middlewareAuthentication(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
void middlewareAuthorization(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
void serverTask(void *params);
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
  secureServer.addMiddleware(&middlewareAuthentication);
  secureServer.addMiddleware(&middlewareAuthorization);
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

void middlewareAuthentication(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  req->setHeader(HEADER_USERNAME, "");
  req->setHeader(HEADER_GROUP, "");
  std::string reqUsername = req->getBasicAuthUser();
  std::string reqPassword = req->getBasicAuthPassword();
  if (reqUsername.length() > 0 && reqPassword.length() > 0) {
    bool authValid = true;
    std::string group = "";
    if (reqUsername == WEBUSER && reqPassword == WEBPASS) {
      group = "ADMIN";
    } else {
      authValid = false;
    }
    if (authValid) {
      req->setHeader(HEADER_USERNAME, reqUsername);
      req->setHeader(HEADER_GROUP, group);
      next();
    } else {
      res->setStatusCode(401);
      res->setStatusText("Unauthorized");
      res->setHeader("Content-Type", "text/plain");
      res->setHeader("WWW-Authenticate", "Basic realm=\"AQM-MAIN\"");
      res->println("401 Unauthorized");
    }
  } else {
    next();
  }
}

void middlewareAuthorization(HTTPRequest * req, HTTPResponse * res, std::function<void()> next) {
  std::string username = req->getHeader(HEADER_USERNAME);
  // we implement authentication for the "/update" path
  if (username == "" && req->getRequestString().substr(0,9) == "/update") {
    res->setStatusCode(401);
    res->setStatusText("Unauthorized");
    res->setHeader("Content-Type", "text/plain");
    res->setHeader("WWW-Authenticate", "Basic realm=\"AQM-MAIN\"");
    res->println("401 Unauthorized");
  } else {
    next();
  }
}

// set configured pass and user from credentials.h
void setupBasicWebAuth(const char* user, const char* pass) {
  WEBUSER = user;
  WEBPASS = pass;
}

// this is true if ESP is initiating an update
bool checkOTAStatus() {
  return OTAinit;
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  res->setHeader("Content-Type", "text/html");
  res->println(aqmIndex);
}

void handleUpdate(HTTPRequest * req, HTTPResponse * res) {
  if (req->getHeader(HEADER_GROUP) == "ADMIN") {
    res->println(updateIndex);
  }
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
