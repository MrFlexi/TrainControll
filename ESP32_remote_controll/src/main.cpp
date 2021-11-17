#include <Arduino.h>

#include <Arduino.h>
#include "WiFi.h"
#include "esp_system.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <SocketIoClient.h>
#include <ArduinoJson.h>


int     lv_i                      = 0;
String  TimeBudget                = "Budget";
String  gs_ad_1                   = "no value";
int     gv_speed = 0;                // the average


//------------------------------------------------------------
// include the ResponsiveAnalogRead library
//------------------------------------------------------------
#include <ResponsiveAnalogRead.h>

// define the pin you want to use
const int ANALOG_PIN = A0;
ResponsiveAnalogRead analog(ANALOG_PIN, true);

#define USE_SERIAL Serial

//------------------------------------------------------------
// WebSocket und Socket IO
//------------------------------------------------------------
ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

//unsigned long previousMillis = 0;
//long interval = 5000;
//unsigned long lastreply = 0;
//unsigned long lastsend = 0;
String str1;


//------------------------------------------------------------
// Socket IO event handler
//------------------------------------------------------------

void event_server_response(const char * payload, size_t length) {
  USE_SERIAL.printf("Server response: %s\n", payload);
}


void event_config_data(const char * payload, size_t length) {
  USE_SERIAL.printf("Config Data: %s\n", payload);

  StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& values = jsonBuffer.createObject();

  JsonObject& root = jsonBuffer.parseObject(payload);

  jsonBuffer.clear();
}

void event_connect(const char * payload, size_t length) {
  USE_SERIAL.printf("I am conected: %s\n");
}


//------------------------------------------------------------
// SETUP
//------------------------------------------------------------

void setup() {
  USE_SERIAL.begin(115200);

  USE_SERIAL.setDebugOutput(true);

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("MrFlexi", "Linde-123");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  Serial.println("Register Events");
  webSocket.on("server_response", event_server_response);
  webSocket.on("config_data", event_config_data);
  webSocket.on("connect", event_connect);
  webSocket.begin("192.168.178.100", 3033);
}
void loop() {
  webSocket.loop();
  analog.update();

  int remainingTimeBudget = ui.update();
  TimeBudget = String( remainingTimeBudget );

  // if the responsive value has change, print out 'changed'
  if (analog.hasChanged()) {

    gv_speed = analog.getValue();
    Serial.print("\tchanged"); Serial.print(analog.getValue());

    //Convert JSON Object to String
    Serial.print("\t");
    Serial.print("Value Changed:");

    //Convert JSON Object to String

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& values = jsonBuffer.createObject();

    values["id"] = analog.getValue();
    values["speed"] = analog.getValue();
    values["dir"] = 2;
    values["f1"] = 0;

    JsonArray& data = root.createNestedArray("data");
    data.add(values);
    //root.printTo(Serial);

    str1 = "";
    root.printTo(str1);
    const char* output = str1.c_str();
    webSocket.emit("main_controller_value_changed", output );
    delay(20);
    jsonBuffer.clear();
  }
}

