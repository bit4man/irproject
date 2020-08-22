// IR Project
// This code is without warrenty and public domain

#define _IR_ENABLE_DEFAULT_ false
#define DECODE_SAMSUNG true
#define SEND_SAMSUNG true

#include <ESP8266WebServer.h>
#include <Uri.h>
#include <ESP8266mDNS.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <PolledTimeout.h>
#include <Wire.h>
#include <Ticker.h>
Ticker ticker;
#include "ircodes.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Samsung.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

#define IRLED D5  // GPIO 14 (D5)
IRsend irsend(IRLED);

ESP8266WebServer server(80);

String inputString = "";
bool stringComplete = false;

void tick() {
  digitalWrite(LED, !digitalRead(LED));
}

void announce(String str) {
  // Serial.println(str);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  announce("Entered config mode");
  //announce(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  announce(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void handleRoot() {
  server.send(200, "text/html",
  "<html>\
    <body style='background: black; overflow: hidden;'>\
        <div style='width: 100vw; height: 100vh; display: grid; place-items: center; color: white;'>\
        Ayyo was good\
        </br>this post brought to you by @emacs arduino-mode\
        </br>\
        <button type='button' onclick='toggleTV();'>Toggle TV</button>\
        <script>\
            function toggleTV() {\
              let xhttp = new XMLHttpRequest();\
              xhttp.onreadystatechange = function() {\
                if (this.readyState == 4 && this.status == 200) {\
                }\
              };\
              xhttp.open('POST', 'toggletv', true);\
              xhttp.send();\
            };\
        </script>\
      </div>\
     </body>\
   </html>");
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

void toggleTV() {
   irsend.sendSAMSUNG(SamsungPowerToggle);
   // Serial.println("Sending toggle IR signal.");
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
    // Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  // Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED off
  digitalWrite(LED, HIGH);

  if (MDNS.begin("remoteoff")) {
    // Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/toggletv", toggleTV);
  server.onNotFound(handleNotFound);
  server.begin();
  
  announce("IR - Connected");
}

void handleSerial() {
  if (Serial.available()) {
    digitalWrite(LED_BUILTIN, HIGH);
    inputString = Serial.readString();
    stringComplete = true;
  }
  if (stringComplete) {
    inputString.trim();
    if (inputString == "toggle" || inputString == "ߗ") {
      toggleTV();
    } else {
      // Serial.println("Unrecognized command: " + inputString);
      while (Serial.available() > 0) {
        Serial.read();
      }
   }
    digitalWrite(LED_BUILTIN, LOW);
    stringComplete = false;
    inputString = "";
  }
}

void setup() {
  Serial.begin(115200);
  // Serial.println();
  pinMode(IRLED, OUTPUT); 

  inputString.reserve(200);

  // initWiFi();
  irsend.begin();
}

void loop() {
  using periodic = esp8266::polledTimeout::periodicMs;
  static periodic nextPing(30000);

  handleSerial();

  // server.handleClient();
  // MDNS.update();
}
