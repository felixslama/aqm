#include <Update.h>
#include "Pages.h"
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

using namespace httpsserver;

#include "cert.h"
#include "private_key.h"

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
// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handleUpdate(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);
void handleDoUpdate(HTTPRequest * req, HTTPResponse * res);
void serverTask(void *params);
void initHTTPS(){
  Serial.println("init");
  xTaskCreatePinnedToCore(serverTask, "https443", 6144, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void serverTask(void *params){
  Serial.println("woah 1");
  /*
  cert = new SSLCert();
  Serial.println("woah 2");
  
  int createCertResult = createSelfSignedCert(
    *cert,
    KEYSIZE_2048,
    "CN=myesp32.local,O=FancyCompany,C=DE",
    "20190101000000",
    "20300101000000"
  ); 
  SSLCert startCert = SSLCert(
    cert->getCertData(),cert->getCertLength(),
    cert->getPKData(),cert->getPKLength()
  ); */
  Serial.println("woah 3");
  /*
  if (createCertResult != 0) {
    Serial.printf("Cerating certificate failed. Error Code = 0x%02X, check SSLCert.hpp for details", createCertResult);
    return;
  }else{
    Serial.println("woah good cert");
  }*/
  
  Serial.println("Creating the certificate was successful");
  
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);
  ResourceNode * nodeUpdate  = new ResourceNode("/update", "GET",&handleUpdate);
  ResourceNode * nodeUpdatePost = new ResourceNode("/update", "POST",&handleDoUpdate);
  ResourceNode * nodeUpdatePut = new ResourceNode("/update", "PUT",&handleDoUpdate);
  
  // Add the root node to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeUpdate);
  secureServer.registerNode(nodeUpdatePost);
  secureServer.registerNode(nodeUpdatePut);
  // Add the 404 not found node to the server.
  secureServer.setDefaultNode(node404);
  
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

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println(aqmIndex);
}
void handleUpdate(HTTPRequest * req, HTTPResponse * res) {
  res->println(updateIndex);
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");

  
}
void handleDoUpdate(HTTPRequest * req, HTTPResponse * res){
  res->setHeader("Content-Type", "text/html");
  byte buffer [256];
  while(!(req->requestComplete())){
    size_t s = req->readBytes(buffer,256);
    res->write(buffer,s);
    res->println(s);
    Serial.println("loop");
  }
  Serial.println("no while loop");
}