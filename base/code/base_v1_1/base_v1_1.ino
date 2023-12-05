#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ra01S.h>
#include <SPI.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "INFINITUM50A3_2.4"
#define WIFI_PASSWORD "qbJQ52tb6e"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCILeO3Adsm2OJ4ZsJhVQujb3IL3WEnaXw"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "gas@test.con"
#define USER_PASSWORD "12345678"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://fir-panel-6af08-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;
// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
//String tempPath = "/id";
String humPath = "/battery";
String presPath = "/level";
String signalPath = "/signal";
String notePath = "/note";
String timePath = "/timestamp";
// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
FirebaseJson json;

//#define RF_FREQUENCY                                433000000 // Hz  center frequency
//#define RF_FREQUENCY                                866000000 // Hz  center frequency
#define RF_FREQUENCY                                915000000 // Hz  center frequency
#define TX_OUTPUT_POWER                             22        // dBm tx output power
#define LORA_BANDWIDTH                              4         // bandwidth
                                                              // 2: 31.25Khz
                                                              // 3: 62.5Khz
                                                              // 4: 125Khz
                                                              // 5: 250KHZ
                                                              // 6: 500Khz 
#define LORA_SPREADING_FACTOR                       7         // spreading factor [SF5..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]

#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_PAYLOADLENGTH                          0         // 0: Variable length packet (explicit header)
                                                              // 1..255  Fixed length packet (implicit header)

SX126x  lora(5,               //Port-Pin Output: SPI select
             4,               //Port-Pin Output: Reset 
             16                //Port-Pin Input:  Busy
             );

const char* ntpServer = "pool.ntp.org";
// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

unsigned long getTime() {
  //timeClient.update();
  time_t now=timeClient.getEpochTime();
  return now;
}


void setup() 
{
  delay(1000);
  Serial.begin(115200);
  //SPI.begin(18, 5, 12, 17);
  SPI.begin();
  //lora.DebugPrint(true);

  int16_t ret = lora.begin(RF_FREQUENCY,              //frequency in Hz
                           TX_OUTPUT_POWER);          //tx power in dBm
  if (ret != ERR_NONE) while(1) {delay(1);
  Serial.println("fuck");}

  lora.LoRaConfig(LORA_SPREADING_FACTOR, 
                  LORA_BANDWIDTH, 
                  LORA_CODINGRATE, 
                  LORA_PREAMBLE_LENGTH, 
                  LORA_PAYLOADLENGTH, 
                  true,               //crcOn  
                  false);             //invertIrq

initWiFi();
timeClient.begin();
timeClient.setTimeOffset(0);
timeClient.update();

 // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  //parentPath= databasePath + "/";
}

void loop() 
{
  uint8_t rxData[10];
  uint8_t rxLen = lora.Receive(rxData, 10);
  if ( rxLen > 0 )
  { 
    Serial.print("Receive rxLen:");
    Serial.println(rxLen);
    for(int i=0;i< rxLen;i++) {
      Serial.print(rxData[i], HEX);
      Serial.print(" ");
    }
    lora.Send(rxData, 4, SX126x_TXMODE_SYNC);
    if (rxLen==4){
      //lora.Send(rxData, 4, SX126x_TXMODE_SYNC);
      json.set(notePath.c_str(), "new");
      Serial.println("new device registered");
    }
    else if (rxLen==9){
      json.set(notePath.c_str(), String(rxData[8]));
      Serial.println("alert!");
    }
    else{
      json.set(notePath.c_str(), "");
    }
    Serial.println("Sensor ID:");
    char a = rxData[0];
    Serial.print(a);
    char b = rxData[1];
    Serial.print(b);
    char c = rxData[2];
    Serial.print(c);
    Serial.println(rxData[3]);
    Serial.println("Sensor value:");
    uint16_t myVar = (rxData[4] << 8) | rxData[5];
    Serial.println(myVar);
    //myVar=0;
    Serial.println("Battery level:");
    uint16_t myVar2 = (rxData[6] << 8) | rxData[7];
    Serial.println(myVar2);

    int8_t rssi, snr;
    lora.GetPacketStatus(&rssi, &snr);
    Serial.print("rssi: ");
    Serial.print(rssi, DEC);
    Serial.println(" dBm");
    Serial.print("snr: ");
    Serial.print(snr, DEC);
    Serial.println(" dB");
    
    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);
    
    //json.set("Ts/.sv", "timestamp"); 
    //json.set(tempPath.c_str(), (String(a)+String(b)+String(c)+String(rxData[3])));
    //json.set(tempPath.c_str(), String(timestamp));
    json.set(humPath.c_str(), String(myVar2));
    json.set(presPath.c_str(), String(myVar));
    json.set(signalPath.c_str(), "rssi="+String(rssi)+"snr="+String(snr));
    json.set(timePath, String(timestamp));
    //parentPath= databasePath + "/" + String(timestamp);
    parentPath= databasePath + "/" + (String(a)+String(b)+String(c)+String(rxData[3])) + "/" + String(timestamp);
    //parentPath=parentPath+"/timestamp";
    //nodeName = String(millis());
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
  delay(1);
}
