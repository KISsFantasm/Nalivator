// Import required libraries

#include "ESPAsyncWebServer.h"

#include "AsyncJson.h"
#include "ArduinoJson.h"

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <FS.h>
  #include <ArduinoOTA.h>
#else
  #include <ESPmDNS.h>
  #include "SPIFFS.h"
  #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

#include "GyverTimer.h"
#include <WiFiUdp.h>


// Replace with your network credentials
// const char* ssid = "ACS-WIFI";
// const char* password = "asuandkvpia";
const char* ssid = "VolumeMeter";
const char* password = "asutp1175";
GTimer readTimer(MS, 750);

const int REG = 0;               // Modbus Hreg Offset
IPAddress remote(192, 168, 11, 137);
// IPAddress remote(192, 168, 11, 178);  // Address of Modbus Slave device

IPAddress ip(192,168,11,150);
IPAddress gateway(192,168,11,1);
// IPAddress ip(192,168,10,205);
// IPAddress gateway(192,168,10,1);
IPAddress subnet(255,255,255,0);

ModbusIP mb;  //ModbusIP object

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const int num = 31;
uint16_t res[num];

int car_section=9;
const char* car_number = "BO0990OB";
bool plcConnect = false;

// #0 8bit - progStep; 8bit - Empty;
// #1 8bit - buttons; 8bit - Empty;
// #2 8bit - Empty; 8bit - Empty;
// #3 2byte - Empty;
// #4,5 4byte - Empty;
// #6,7 4byte - fcount01;
// #8,9 4byte - fcount02;
// #10,11 4byte - fcount03;
// #12,13 4byte - fcount04;
// #14 2byte - mode;
// #15 2byte - memMode;
// #16 2byte - maxFlow;
// #17 2byte - gPImp;
// #18 2byte - maxVOut;
// #19 2byte - midVOut;
// #20 2byte - minVOut;
// #21 2byte - minKg;
// #22 2byte - drainageTime;
// #23 2byte - prepareTime;
// #24,25 4byte - Empty;
// #26,27 4byte - ftCounter;
// #28,29 4byte - counterMem;
// #30 8bit - currentPos; 8bit - Empty;

void writeParamToPLC (int mbIdx, uint16_t val){
  mb.writeHreg(remote, mbIdx, val, NULL, 1);
}

void setPlcParam(StaticJsonDocument<512> json){
  if (json["startCButton"]) {if(json["startCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<0);} else {writeParamToPLC(1, res[1]&=~(1<<0));}}
  if (json["stopCButton"]) {if(json["stopCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<1);} else {writeParamToPLC(1, res[1]&=~(1<<1));}}
  if (json["startSButton"]) {if(json["startSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<2);} else {writeParamToPLC(1, res[1]&=~(1<<2));}}
  if (json["stopSButton"]) {if(json["stopSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<3);} else {writeParamToPLC(1, res[1]&=~(1<<3));}}
  if (json["zeroButton"]) {if(json["zeroButton"] == "on"){writeParamToPLC(1, res[1]|=1<<4);} else {writeParamToPLC(1, res[1]&=~(1<<4));}}
  if (json["doseButton"]) {if(json["doseButton"] == "on"){writeParamToPLC(1, res[1]|=1<<5);} else {writeParamToPLC(1, res[1]&=~(1<<5));}}
  if (json["flowButton"]) {if(json["flowButton"] == "on"){writeParamToPLC(1, res[1]|=1<<6);} else {writeParamToPLC(1, res[1]&=~(1<<6));}}
  if (json["param_0"]) {writeParamToPLC(16, json["param_0"]);}
  if (json["param_1"]) {writeParamToPLC(17, json["param_1"]);}
  if (json["param_2"]) {writeParamToPLC(18, json["param_2"]);}
  if (json["param_3"]) {writeParamToPLC(19, json["param_3"]);}
  if (json["param_4"]) {writeParamToPLC(20, json["param_4"]);}
  if (json["param_5"]) {writeParamToPLC(21, json["param_5"]);}
  if (json["param_6"]) {writeParamToPLC(22, json["param_6"]);}
  if (json["param_7"]) {writeParamToPLC(23, json["param_7"]);}
  if (json["car_section"]) {car_section = json["car_section"];}
  if (json["car_number"]) {car_number = json["car_number"];}
  if (json["option"]) {writeParamToPLC(14, json["option"]);}
}

StaticJsonDocument<512> getJsonPlcData(bool needAllData){
  StaticJsonDocument<512> json;
  json["val_0"] = ((uint32_t)res[7] << 16) | res[6];
  json["val_1"] = ((uint32_t)res[9] << 16) | res[8];
  json["val_2"] = ((uint32_t)res[11] << 16) | res[10];
  json["val_3"] = ((uint32_t)res[13] << 16) | res[12];
  json["progStep"] = res[0];
  json["current"] = res[30];
  json["option"] = res[14];
  json["plc_connect"] = plcConnect;
  if (needAllData){
    json["param_0"] = res[16];
    json["param_1"] = res[17];
    json["param_2"] = res[18];
    json["param_3"] = res[19];
    json["param_4"] = res[20];
    json["param_5"] = res[21];
    json["param_6"] = res[22];
    json["param_7"] = res[23];
    json["car_number"] = car_number;
    json["car_section"] = car_section;
  }
  return json;
}

void setup(){
  // Initialize SPIFFS
  #ifdef ESP8266
    Serial.begin(74880);
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    Serial.begin(115200);
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // if (readTimer.isReady()) { 
      Serial.print(".");
    // }
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String());
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "image/png");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/Connect", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(getJsonPlcData(false), *response);
      request->send(response);
    },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {request->send(200, "text/plain", "error");},
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      StaticJsonDocument<512> json;
      deserializeJson(json, data);
      setPlcParam(json);
      // if (json["needAllData"]) {needAllData = true;} else {needAllData = false;}
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      serializeJson(getJsonPlcData(json["needAllData"]), *response);
      request->send(response);
    }
  );
  #ifdef ESP8266
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
  #endif
  // Start server
  server.begin();
  mb.client();
}

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus Transaction callback
  if (event != Modbus::EX_SUCCESS)                  // If transaction got an error
    Serial.printf("Modbus result: %02X\n", event);  // Display Modbus error code
  if (event == Modbus::EX_TIMEOUT) {    // If Transaction timeout took place
    mb.disconnect(remote);              // Close connection to slave and
    mb.dropTransactions();              // Cancel all waiting transactions
  }
  return true;
}

void loop(){

  #ifdef ESP8266
    ArduinoOTA.handle();
  #endif

  if (readTimer.isReady()) {
    if (plcConnect = mb.isConnected(remote)) {
      mb.readHreg(remote, REG, res, num, cb, 1);
    } else {
      mb.connect(remote);
    }
    mb.task();
  }
}
