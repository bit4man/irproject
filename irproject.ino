// IR Project
// This code is without warrenty and public domain

#define _IR_ENABLE_DEFAULT_ false
#define DECODE_NEC true
#define SEND_NEC true

#include <ESP8266WebServer.h>
#include <Uri.h>
#include <ESP8266mDNS.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <PolledTimeout.h>
#include <Wire.h>
#include "SSD1306Wire.h" 
#include <Ticker.h>
Ticker ticker;
#include "ircodes.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_NEC.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

#define IRLED D5  // GPIO 14 (D5)
IRsend irsend(IRLED);

ESP8266WebServer server(80);

SSD1306Wire display(0x3c, SDA, SCL); // ADDRESS, SDA, SCL
// ESP8266/D1 Mini SDA=D2, SCL=D1

int _lineno = 0;

void tick() {
  digitalWrite(LED, !digitalRead(LED));
}

void announce(String str, int line) {
  Serial.println(str);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 12*line, str);
}

void announce(String str) {
  announce(str, _lineno++);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  _lineno=0; display.clear();
  announce("Entered config mode");
//  announce(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  announce(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
  display.display();
}


void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!\r\n");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void turnOff() {
   irsend.sendNEC(NECPowerToggle);
   server.send(200, "text/plain", "Sent IR command");
}

void initWiFi() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  pinMode(LED, OUTPUT);
  ticker.attach(0.6, tick);

  WiFiManager wm;
  //reset settings - for testing
//   wm.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(LED, LOW);

  if (MDNS.begin("remoteoff")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/turnoff", turnOff);
  server.onNotFound(handleNotFound);
  server.begin();
  
  _lineno=1;
  display.clear();
  announce("IR - Connected");
  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(IRLED, OUTPUT); 

  // display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5,10, "IR Shutoff");
  display.display();
  delay(500);

  initWiFi();
  irsend.begin();
}

void loop() {
  using periodic = esp8266::polledTimeout::periodicMs;
  static periodic nextPing(30000);

  server.handleClient();
  MDNS.update();
  if (nextPing) {
      irsend.sendNEC(NECPowerToggle);
  }
}
