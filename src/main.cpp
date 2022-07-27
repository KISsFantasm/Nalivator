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
#include "string"

// Wifi

// const char* ssid = "ACS-WIFI";
// const char* password = "asuandkvpia";

const char* http_username[] = {"admin","operator","guest"};
const char* http_password[] = {"","",""};

// #ifdef ESP8266
//   short wifiIp[4] = {192,168,10,198};
// #else
//   short wifiIp[4] = {192,168,10,205}; 
// #endif

// short wifiGateway[4] = {192,168,10,1}; 
// short wifiSubnet[4] = {255,255,255,0}; 

// AP

// const char* espSsid = "ESP_default_4_150";
// const char* espPassword = "espdefault";

// short apIp[4] = {192,168,4,150}; 
// short apGateway[4] = {192,168,4,1}; 
// short apSubnet[4] = {255,255,255,0}; 

//Timer
GTimer readTimer(MS, 750);

// Modbus Hreg Offset
const int REG = 0;               
IPAddress remote(192, 168, 10, 178);  // Address of Modbus Slave device
// IPAddress remote(192, 168, 10, 224);
ModbusIP mb;  //ModbusIP object

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

bool plcConnect = false;

short drops = 0;
short gdrops = 0;

struct memSt {
  short stepMem;
  short carS;
  char carN[11];
  short wIP[4];
  short wGate[4];
  short wMask[4];
  char wSsid[16];
  char wPass[16];
  short apIP[4];
  short apGate[4];
  short apMask[4];
  char apSsid[16];
  char apPass[16];
  bool apEnable;
} memParam;

String fileName = "main";

const int num = 25;
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
uint16_t res[num];

void writeParamToPLC (int mbIdx, uint16_t val){
  mb.writeHreg(remote, mbIdx, val, NULL, 1);
}

void inline writeMemStruct(){
  EEPROM.put(0, memParam);
  EEPROM.commit();
}

StaticJsonDocument<512> jsonIn;
void setPlcParam(){
  if (jsonIn["startCButton"]) {if(jsonIn["startCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<0);} else {writeParamToPLC(1, res[1]&=~(1<<0));}}
  if (jsonIn["stopCButton"]) {if(jsonIn["stopCButton"] == "on"){writeParamToPLC(1, res[1]|=1<<1);} else {writeParamToPLC(1, res[1]&=~(1<<1));}}
  if (jsonIn["startSButton"]) {if(jsonIn["startSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<2);} else {writeParamToPLC(1, res[1]&=~(1<<2));}}
  if (jsonIn["stopSButton"]) {if(jsonIn["stopSButton"] == "on"){writeParamToPLC(1, res[1]|=1<<3);} else {writeParamToPLC(1, res[1]&=~(1<<3));}}
  if (jsonIn["zeroButton"]) {if(jsonIn["zeroButton"] == "on"){writeParamToPLC(1, res[1]|=1<<4);} else {writeParamToPLC(1, res[1]&=~(1<<4));}}
  if (jsonIn["doseButton"]) {if(jsonIn["doseButton"] == "on"){writeParamToPLC(1, res[1]|=1<<5);} else {writeParamToPLC(1, res[1]&=~(1<<5));}}
  if (jsonIn["flowButton"]) {if(jsonIn["flowButton"] == "on"){writeParamToPLC(1, res[1]|=1<<6);} else {writeParamToPLC(1, res[1]&=~(1<<6));}}
  if (jsonIn["param_0"]) {writeParamToPLC(12, jsonIn["param_0"]);}
  if (jsonIn["param_1"]) {writeParamToPLC(13, jsonIn["param_1"]);}
  if (jsonIn["param_2"]) {writeParamToPLC(14, jsonIn["param_2"]);}
  if (jsonIn["param_3"]) {writeParamToPLC(15, jsonIn["param_3"]);}
  if (jsonIn["param_4"]) {writeParamToPLC(16, jsonIn["param_4"]);}
  if (jsonIn["param_5"]) {writeParamToPLC(17, jsonIn["param_5"]);}
  if (jsonIn["param_6"]) {writeParamToPLC(18, jsonIn["param_6"]);}
  if (jsonIn["param_7"]) {writeParamToPLC(19, jsonIn["param_7"]);}
  if (jsonIn["car_section"]) {memParam.carS = jsonIn["car_section"]; writeMemStruct();}
  if (jsonIn["car_number"]) {String temp = jsonIn["car_number"]; temp.toCharArray(memParam.carN, 11);  writeMemStruct();}
  if (jsonIn["option"]) {writeParamToPLC(10, jsonIn["option"]);}
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
  // jsonOut["plc_connect"] = plcConnect;
  jsonOut["plc_connect"] = true;
  jsonOut["WiFi_RSSI"] = WiFi.RSSI();
  if (jsonIn["needAllData"] && jsonIn["needAllData"] == true) {
    jsonIn["needAllData"] = false;
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

String inline getAuthHeaderValue(String header, String value){
  String str = "";
  int f1=0, f2=0, f3=0;
  f1 = header.indexOf(value+"=");
  if (f1 != -1){
    f2 = header.indexOf("\"",f1)+1;
    f3 = header.indexOf("\"",f2);
    str = header.substring(f2,f3);
  }
  return str;
}

bool isAuth = false;
bool inline isAuthorized(AsyncWebServerRequest *request, bool skip = false){
  if (!skip) isAuth = false;
  bool valid = false;
  for (short i = 0; i < (*(&http_username + 1) - http_username); i++){
    if(request->authenticate(http_username[i], http_password[i])) return true;
  }
  return false;
}

bool inline getIOSAuth(AsyncWebServerRequest *request){
  String str;
  bool hasAuthH = false;
  bool hasRefH = false;
  for (short i=0;i<request->headers();i++) {
    if(request->getHeader(i)->name() == "User-Agent") str = request->getHeader(i)->toString();
    if(request->getHeader(i)->name() == "Authorization") hasAuthH = true;
  }
  if (str.indexOf("iPhone OS") != -1 || str.indexOf("Mac OS") != -1) {
    if (!isAuthorized(request,true)){
      request->requestAuthentication();
      return false;
    }
  }
  return true;
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

  EEPROM.begin(140);
  EEPROM.get(0, memParam);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_AP_STA);
  // WiFi.begin(ssid, password);
  // WiFi.config(
  //   IPAddress(wifiIp[0],wifiIp[1],wifiIp[2],wifiIp[3]),
  //   IPAddress(wifiGateway[0],wifiGateway[1],wifiGateway[2],wifiGateway[3]),
  //   IPAddress(wifiSubnet[0],wifiSubnet[1],wifiSubnet[2],wifiSubnet[3])
  // );

  WiFi.begin(memParam.wSsid, memParam.wPass);
  WiFi.config(
    IPAddress(memParam.wIP[0],memParam.wIP[1],memParam.wIP[2],memParam.wIP[3]),
    IPAddress(memParam.wGate[0],memParam.wGate[1],memParam.wGate[2],memParam.wGate[3]),
    IPAddress(memParam.wMask[0],memParam.wMask[1],memParam.wMask[2],memParam.wMask[3])
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

  WiFi.enableAP(memParam.apEnable);
  if(memParam.apEnable){
    WiFi.softAP(memParam.apSsid, memParam.apPass);
    WiFi.softAPConfig(
      IPAddress(memParam.apIP[0],memParam.apIP[1],memParam.apIP[2],memParam.apIP[3]),
      IPAddress(memParam.apGate[0],memParam.apGate[1],memParam.apGate[2],memParam.apGate[3]),
      IPAddress(memParam.apMask[0],memParam.apMask[1],memParam.apMask[2],memParam.apMask[3])
    );
  }

  // Serial.println("");
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    getIOSAuth(request);
    if (!isAuthorized(request)) {request->redirect("/LogIn"); return;}
    String userName;
    for (short i = 0; i < request->headers(); i++) {
      if (request->headerName(i) == "Authorization"){
        userName = getAuthHeaderValue(request->header(i), "username");
      }
    }
    String uri;
    if (userName == "admin") uri = "/setting.html"; else uri = "/main.html";  
    request->send(SPIFFS, uri, "text/html");
  });

  server.on("^\\/[\\w]+\\.(js|css|ico)$", HTTP_GET, [](AsyncWebServerRequest *request){
    getIOSAuth(request);
    if (!isAuthorized(request)) {request->redirect("/LogIn"); return;}
    String userName,uri,ref;
    for (short i = 0; i < request->headers(); i++) {
      if (request->headerName(i) == "Authorization"){
        userName = getAuthHeaderValue(request->header(i), "username");
        uri = getAuthHeaderValue(request->header(i), "uri");
        uri = uri.substring(uri.indexOf("."),uri.length());
      }
      if (request->headerName(i) == "Referer"){
        ref = request->header(i);
      }
    }
    if (uri && ref != ""){
      if (userName == "admin") uri = "/setting"+uri; else uri = "/main"+uri;  
      request->send(SPIFFS, uri, "text/javascript");
    } else {
      request->send(400);
    }
  });

  server.on("/LogIn", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!getIOSAuth(request)) return;
    if (!isAuth){
      isAuth = true;
      request->requestAuthentication();
    }
    request->redirect("/");
  });

  server.on("/ESP", HTTP_POST, [](AsyncWebServerRequest *request) {

    String userName;
    bool auth = false;
    for (short i = 0; i < request->headers(); i++) {
      if (request->headerName(i) == "Authorization"){
        auth = true;
        userName = getAuthHeaderValue(request->header(i), "username");
      }
    }
    
    if (!auth) return request->requestAuthentication();

    bool setPlcParab = false;
    for (size_t  i = 0 ; i < request->args(); i++){
      AsyncWebParameter *par = request->getParam(i);
      if (par->name() == "json" ) {
        setPlcParab = true;
        deserializeJson(jsonIn, par->value());
      }
    }

    getJsonPlcData();
    if(setPlcParab){
      if (userName != "guest") setPlcParam();
      jsonIn = {};
    }

    AsyncResponseStream *response = request->beginResponseStream("application/json");

    jsonOut["access"] = userName;
    serializeJson(jsonOut, *response);
    request->send(response);
  });

  server.on("/settingLoad", HTTP_POST, [](AsyncWebServerRequest *request) {
    String str;
    for(short i=0;i<4;i++){
      str+=(String)memParam.wIP[i]+"-"+memParam.wGate[i]+"-"+memParam.wMask[i]+"-"+memParam.apIP[i]+"-"+memParam.apGate[i]+"-"+memParam.apMask[i]+"|";
    }
    str+=(String)memParam.apEnable+"|"+memParam.wSsid+"|"+memParam.wPass+"|"+memParam.apSsid+"|"+memParam.apPass;
    request->send(200, String(), str);
  });

  server.on("/settingSave", HTTP_POST, [](AsyncWebServerRequest *request) {
    for (size_t  i = 0 ; i < request->args(); i++){
      AsyncWebParameter *par = request->getParam(i);
      if (par->name() == "paramStr") {
        String str = par->value();
        String val, num;
        short idx, k;
        for(short i=0;i<6;i++){
          idx = str.indexOf("|");
          val = str.substring(0, idx);
          str = str.substring(idx+1, str.length());
          for(short j=0;j<4;j++){
            k = val.indexOf(".");
            num = val.substring(0, k);
            val = val.substring(k+1, val.length());
            switch(i){
              case 0: memParam.wIP[j] = num.toInt(); break;
              case 1: memParam.wGate[j] = num.toInt(); break;
              case 2: memParam.wMask[j] = num.toInt(); break;
              case 3: memParam.apIP[j] = num.toInt(); break;
              case 4: memParam.apGate[j] = num.toInt(); break;
              case 5: memParam.apMask[j] = num.toInt(); break;
            }
          }
        }
        for(short i=0;i<5;i++){
          idx = str.indexOf("|");
          val = str.substring(0, idx);
          str = str.substring(idx+1, str.length());
          switch(i){
            case 0: memParam.apEnable = val == "true" ? true : false; break;
            case 1: val.toCharArray(memParam.wSsid,16); break;
            case 2: val.toCharArray(memParam.wPass,16); break;
            case 3: val.toCharArray(memParam.apSsid,16); break;
            case 4: val.toCharArray(memParam.apPass,16); break;
          }
        }
        writeMemStruct();
        delay(2000);
        ESP.restart();
      }
    }
    request->send(200);
  });

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
