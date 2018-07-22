#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SocketIoClient.h>

#include "SH1106.h"
#include "SH1106Ui.h"
#include "images.h"
#include <ArduinoJson.h>


//------------------------------------------------------------
// OLED Display Definition
//------------------------------------------------------------

/* Node MCU Hardware SPI Pins  <--> OLED display
  D5 GPIO14   CLK              <--> CLK
  D7 GPIO13   MOSI (DOUT)      <--> Data In
*/
#define OLED_RESET  D6   // RESET
#define OLED_DC     D2   // Data/Command
#define OLED_CS     D8   // Chip select

SH1106 display(true, OLED_RESET, OLED_DC, OLED_CS); // FOR SPI
SH1106Ui ui     ( &display );

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
// Arduino JSON library
// https://bblanchon.github.io/ArduinoJson/
//------------------------------------------------------------



//------------------------------------------------------------
// OLED Display Implementation
//------------------------------------------------------------

bool msOverlay(SH1106 *display, SH1106UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);   // Overlay wird gebraucht, sonst werden die Werte auf dem Display nicht aktualisiert
  display->setFont(ArialMT_Plain_10);
  display->drawString(0, 0, "TrainControll V 1.0" );
  
  //display->drawString(0, 0, "ETA 512" + String( gv_sw1 )+ "  " + String( gv_sw2 ));
  //display->drawString(0, 0, "ETA 512" + CL_CTRL.getDirection_frontend_graphic( ));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128, 0, "Speed" + String( gv_speed ));
  return true;
}

bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  // if this frame need to be refreshed at the targetFPS you need to
  // return true
  //display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display->drawXbm(x + 5, y + 14, ice_width, ice_height, ice_bits);
  return false;
}

bool drawFrame2(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Demonstrates the 3 included default sizes. The fonts come from SH1106Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  String ls_ip;
  //ls_ip = WiFi.localIP().toString();
  display->drawString(0 + x, 10 + y, "WebSocketIO");
  //display->drawString(0 + x, 34 + y, ls_ip);

  // if this frame need to be refreshed at the targetFPS you need to
  // return true
  return false;
}

bool drawFrame3(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_16);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 5 + y, "Speed");

  // The coordinates define the right end of the text
  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22, String( gv_speed ));
  return true;
}

bool drawFrame4(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping

  // display->drawXbm(x + 5, y + 10, dampflok_width, dampflok_height, dampflok_bits);

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  //display->drawStringMaxWidth(0 + x, 40 + y, 128, CL_CTRL.getDirection_frontend( ));
  display->drawXbm(x + 5, y + 14, dampflok_width, dampflok_height, dampflok_bits);
  return true;
}

// how many frames are there?
int frameCount = 4;
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };

bool (*overlays[])(SH1106 *display, SH1106UiState* state)             = { msOverlay };
int overlaysCount = 1;

void setup_display ()
{
  // Setup Display
  ui.setTargetFPS(30);
  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Inital UI takes care of initalising the display too.
  ui.init();

  //display.flipScreenVertically();
}




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

  // Initialize OLED Display
  setup_display();

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
  webSocket.begin("192.168.1.144", 3000);
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

    values["lok_speed"] = analog.getValue();
    values["lok_dir"] = 2;
    values["lok_f1"] = 0;

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

