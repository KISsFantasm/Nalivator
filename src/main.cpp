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

#include <EEPROM.h>

// Wifi

const char* ssid = "ACS-WIFI";
const char* password = "asuandkvpia";

const char* http_username[] = {"admin","admin2"};
const char* http_password[] = {"admin","admin2"};

#ifdef ESP8266
  short wifiIp[4] = {192,168,10,198};
#else
  short wifiIp[4] = {192,168,10,205}; 
#endif

short wifiGateway[4] = {192,168,10,1}; 
short wifiSubnet[4] = {255,255,255,0}; 

// AP

const char* espSsid = "ESP_Nalivator";
const char* espPassword = "ESPNalivator";

short apIp[4] = {192,168,4,150}; 
short apGateway[4] = {192,168,4,1}; 
short apSubnet[4] = {255,255,255,0}; 

//Timer
GTimer readTimer(MS, 750);

// Modbus Hreg Offset
const int REG = 0;               
IPAddress remote(192, 168, 10, 178);  // Address of Modbus Slave device
// IPAddress remote(192, 168, 10, 224);
ModbusIP mb;  //ModbusIP object

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const int num = 25;
uint16_t res[num];

bool plcConnect = false;

short drops = 0;
short gdrops = 0;

struct memSt {
  short stepMem;
  short carS;
  String carN;
} memParam;

String fileName = "main";

// #0 8bit - progStep; 8bit - Empty;
// #1 8bit - buttons; 8bit - Empty;
// #2,3 4byte - fcount01;
// #4,5 4byte - fcount02;
// #6,7 4byte - fcount03;
// #8,9 4byte - fcount04;
// #10 2byte - mode;
// #11 2byte - memMode;
// #12 2byte - maxFlow;
// #13 2byte - gPImp;
// #14 2byte - maxVOut;
// #15 2byte - midVOut;
// #16 2byte - minVOut;
// #17 2byte - minKg;
// #18 2byte - drainageTime;
// #19 2byte - prepareTime;
// #20,21 4byte - ftCounter;
// #22,23 4byte - counterMem;
// #24 8bit - currentPos; 8bit - Empty;

void writeParamToPLC (int mbIdx, uint16_t val){
  mb.writeHreg(remote, mbIdx, val, NULL, 1);
}

void inline writeMemStruct(){
  EEPROM.put(0, memParam);
  EEPROM.commit();
}

StaticJsonDocument<512> jsonIN;
void setPlcParam(){
  if (jsonIN["startCButton"]) {if(jsonIN["startCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<0);} else {writeParamToPLC(1, res[1]&=~(1<<0));}}
  if (jsonIN["stopCButton"]) {if(jsonIN["stopCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<1);} else {writeParamToPLC(1, res[1]&=~(1<<1));}}
  if (jsonIN["startSButton"]) {if(jsonIN["startSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<2);} else {writeParamToPLC(1, res[1]&=~(1<<2));}}
  if (jsonIN["stopSButton"]) {if(jsonIN["stopSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<3);} else {writeParamToPLC(1, res[1]&=~(1<<3));}}
  if (jsonIN["zeroButton"]) {if(jsonIN["zeroButton"] == "on"){writeParamToPLC(1, res[1]|=1<<4);} else {writeParamToPLC(1, res[1]&=~(1<<4));}}
  if (jsonIN["doseButton"]) {if(jsonIN["doseButton"] == "on"){writeParamToPLC(1, res[1]|=1<<5);} else {writeParamToPLC(1, res[1]&=~(1<<5));}}
  if (jsonIN["flowButton"]) {if(jsonIN["flowButton"] == "on"){writeParamToPLC(1, res[1]|=1<<6);} else {writeParamToPLC(1, res[1]&=~(1<<6));}}
  if (jsonIN["param_0"]) {writeParamToPLC(12, jsonIN["param_0"]);}
  if (jsonIN["param_1"]) {writeParamToPLC(13, jsonIN["param_1"]);}
  if (jsonIN["param_2"]) {writeParamToPLC(14, jsonIN["param_2"]);}
  if (jsonIN["param_3"]) {writeParamToPLC(15, jsonIN["param_3"]);}
  if (jsonIN["param_4"]) {writeParamToPLC(16, jsonIN["param_4"]);}
  if (jsonIN["param_5"]) {writeParamToPLC(17, jsonIN["param_5"]);}
  if (jsonIN["param_6"]) {writeParamToPLC(18, jsonIN["param_6"]);}
  if (jsonIN["param_7"]) {writeParamToPLC(19, jsonIN["param_7"]);}
  if (jsonIN["car_section"]) {memParam.carS = jsonIN["car_section"]; writeMemStruct();}
  if (jsonIN["car_number"]) {String temp = jsonIN["car_number"]; memParam.carN = temp;  writeMemStruct();}
  if (jsonIN["option"]) {writeParamToPLC(10, jsonIN["option"]);}
}

StaticJsonDocument<512> jsonOut;
void getJsonPlcData(){
  jsonOut = {};
  jsonOut["val_0"] = ((uint32_t)res[3] << 16) | res[2];
  jsonOut["val_1"] = ((uint32_t)res[5] << 16) | res[4];
  jsonOut["val_2"] = ((uint32_t)res[7] << 16) | res[6];
  jsonOut["val_3"] = ((uint32_t)res[9] << 16) | res[8];
  jsonOut["progStep"] = res[0];
  jsonOut["current"] = res[24];
  jsonOut["option"] = res[10];
  jsonOut["plc_connect"] = plcConnect;
  jsonOut["WiFi_RSSI"] = WiFi.RSSI();
  if (jsonIN["needAllData"] && jsonIN["needAllData"] == true) {
    jsonIN["needAllData"] = false;
    jsonOut["car_number"] = memParam.carN;
    jsonOut["car_section"] = memParam.carS;
    jsonOut["param_0"] = res[12];
    jsonOut["param_1"] = res[13];
    jsonOut["param_2"] = res[14];
    jsonOut["param_3"] = res[15];
    jsonOut["param_4"] = res[16];
    jsonOut["param_5"] = res[17];
    jsonOut["param_6"] = res[18];
    jsonOut["param_7"] = res[19];
  }
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

  EEPROM.begin(24);
  EEPROM.get(0, memParam);
  // Serial.println("");
  // Serial.println((String) memParam.carN + " - " + memParam.carS + " - " + memParam.stepMem);

  // Connect to Wi-Fi
  // WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(
    IPAddress(wifiIp[0],wifiIp[1],wifiIp[2],wifiIp[3]),
    IPAddress(wifiGateway[0],wifiGateway[1],wifiGateway[2],wifiGateway[3]),
    IPAddress(wifiSubnet[0],wifiSubnet[1],wifiSubnet[2],wifiSubnet[3])
  );

  // WiFi.softAP(espSsid, espPassword);
  // WiFi.softAPConfig(
  //   IPAddress(apIp[0],apIp[1],apIp[2],apIp[3]),
  //   IPAddress(apGateway[0],apGateway[1],apGateway[2],apGateway[3]),
  //   IPAddress(apSubnet[0],apSubnet[1],apSubnet[2],apSubnet[3])
  // );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    bool valid = false;
    for (short i = 0; i < (*(&http_username + 1) - http_username); i++){
      // const char* a = http_username[0];
      // Serial.println((String)http_username[i]+" : "+http_password[i]);
      // Serial.println(request->client()->connected());
      for (short i = 0; i <= request->headers(); i++) {
        Serial.println(request->header(i));
      }
      // request->getHeader()
      if(request->authenticate(http_username[i], http_password[i])){
        // request->client()->
        valid = true;
        break;
      }
    }

    if (!valid) return request->requestAuthentication();
    request->send(SPIFFS, "/main.html", "text/html");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/main.ico", "image/png");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    // Serial.println(request->client()->);
    for (short i = 0; i <= request->headers(); i++) {
      Serial.println(request->header(i));
    }
    request->send(SPIFFS, "/main.css", "text/css");
  });

  // server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/main.js", "text/javascript");
  // });

    server.on("^\\/[\\w]+\\.(js|html)$", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/main.js", "text/javascript");
  });

  server.on("/Connect", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      for (size_t  i = 0 ; i < request->args(); i++){
        AsyncWebParameter *par = request->getParam(i);
        if (par->name() == "json") {
          deserializeJson(jsonIN, par->value());
          setPlcParam();
        }
      }
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      getJsonPlcData();
      serializeJson(jsonOut, *response);
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

StaticJsonDocument<512> jsonSD;
void inline writeToSD() {
  jsonSD = {};
  jsonSD["carN"] = memParam.carN;
  jsonSD["carS"] = memParam.carS;
  switch(res[24]){
    case 0 : jsonSD["val"] = ((uint32_t)res[3] << 16) | res[2];
    break;
    case 1 : jsonSD["val"] = ((uint32_t)res[5] << 16) | res[4];
    break;
    case 2 : jsonSD["val"] = ((uint32_t)res[7] << 16) | res[6];
    break;
    case 3 : jsonSD["val"] = ((uint32_t)res[9] << 16) | res[8];
    break;    
  }
  //...

  //...

  jsonSD = {};
}

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus Transaction callback
  if (event != Modbus::EX_SUCCESS) { // If transaction got an error
    // Serial.printf("Modbus result: %02X\n", event);  // Display Modbus error code
  } else {
    drops = 0;
  }
  if (event == Modbus::EX_TIMEOUT) {    // If Transaction timeout took place
    drops++;
    if (drops >= 5) {
      mb.disconnect(remote);              // Close connection to slave and
      mb.dropTransactions();              // Cancel all waiting transactions
    }
  }

  if (memParam.stepMem == 11 && res[0] != 11){
    writeToSD();
  }

  if (memParam.stepMem != res[0]){
    memParam.stepMem = res[0];
    writeMemStruct();
  }

  return true;
}
    
void loop(){

  #ifdef ESP8266
    ArduinoOTA.handle();
  #endif

  if (readTimer.isReady()) {
    if ((plcConnect = mb.isConnected(remote))) {
      mb.readHreg(remote, REG, res, num, cb, 1);
    } else {
      mb.connect(remote);
    }
    mb.task();
  }
}
