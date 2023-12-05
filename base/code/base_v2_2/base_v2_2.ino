#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#define WIFI_AUTH_OPEN ENC_TYPE_NONE
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

//#include <NTPClient.h>
//#include <WiFiUdp.h>
#include <Ra01S.h>
#include <SPI.h>

// Insert your network credentials
#define WIFI_SSID "INFINITUM50A3_2.4"
#define WIFI_PASSWORD "qbJQ52tb6e"
 #define INFLUXDB_URL "http://3.93.246.85:8086"
  #define INFLUXDB_TOKEN "E7LnF_IXZjUAAgLRrNYfJnXWgKMa_ie3t7kmZ5JNAcJWydrrwtseamCddxxLht-ioq6GvjelcOz2o467v_9ZBw=="
  #define INFLUXDB_ORG "0c414f2ddca87aa5"
  #define INFLUXDB_BUCKET "test"
   // Time zone info
  #define TZ_INFO "UTC-5"
  
  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// NTP servers the for time synchronization.
// For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
#define NTP_SERVER1  "pool.ntp.org"
#define NTP_SERVER2  "time.nis.gov"
#define WRITE_PRECISION WritePrecision::S
#define MAX_BATCH_SIZE 10
#define WRITE_BUFFER_SIZE 30


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

int iterations = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;

ESP8266WebServer server(80);

bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 120 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}
 
void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}
 
void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
 
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("how2electronics", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}
 
void createWebServer()
{
 {
    server.on("/", []() {
 
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
 
      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });
 
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
 
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
 
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
 
    });
  } 
}

uint8_t converter(uint16_t dat){
  uint8_t p=0;
  if (dat<250) p=1;
  else if (dat<322) p=5;
  else if (dat<394) p=6;
  else if (dat<466) p=7;
  else if (dat<538) p=8;
  else if (dat<610) p=9;
  else if (dat<680) p=10;
  else if (dat<728) p=11;
  else if (dat<776) p=12;
  else if (dat<824) p=13;
  else if (dat<872) p=14;
  else if (dat<920) p=15;
  else if (dat<968) p=16;
  else if (dat<1016) p=17;
  else if (dat<1064) p=18;
  else if (dat<1112) p=19;
  else if (dat<1164) p=20;
  else if (dat<1187) p=21;
  else if (dat<1210) p=22;
  else if (dat<1233) p=23;
  else if (dat<1256) p=24;
  else if (dat<1280) p=25;
  else if (dat<1308) p=26;
  else if (dat<1336) p=27;
  else if (dat<1364) p=28;
  else if (dat<1392) p=29;
  else if (dat<1420) p=30;
  else if (dat<1448) p=31;
  else if (dat<1476) p=32;
  else if (dat<1504) p=33;
  else if (dat<1532) p=34;
  else if (dat<1560) p=35;
  else if (dat<1588) p=36;
  else if (dat<1616) p=37;
  else if (dat<1644) p=38;
  else if (dat<1672) p=39;
  else if (dat<1700) p=40;
  else if (dat<1732) p=41;
  else if (dat<1764) p=42;
  else if (dat<1796) p=43;
  else if (dat<1828) p=44;
  else if (dat<1860) p=45;
  else if (dat<1882) p=46;
  else if (dat<1904) p=47;
  else if (dat<1926) p=48;
  else if (dat<1948) p=49;
  else if (dat<1970) p=50;
  else if (dat<2001) p=51;
  else if (dat<2032) p=52;
  else if (dat<2063) p=53;
  else if (dat<2094) p=54;
  else if (dat<2125) p=55;
  else if (dat<2149) p=56;
  else if (dat<2173) p=57;
  else if (dat<2197) p=58;
  else if (dat<2221) p=59;
  else if (dat<2245) p=60;
  else if (dat<2276) p=61;
  else if (dat<2307) p=62;
  else if (dat<2338) p=63;
  else if (dat<2369) p=64;
  else if (dat<2400) p=65;
  else if (dat<2428) p=66;
  else if (dat<2456) p=67;
  else if (dat<2484) p=68;
  else if (dat<2512) p=69;
  else if (dat<2540) p=70;
  else if (dat<2572) p=71;
  else if (dat<2604) p=72;
  else if (dat<2636) p=73;
  else if (dat<2668) p=74;
  else if (dat<2700) p=75;
  else if (dat<2728) p=76;
  else if (dat<2756) p=77;
  else if (dat<2784) p=78;
  else if (dat<2812) p=79;
  else if (dat<2840) p=80;
  else if (dat<2884) p=81;
  else if (dat<2928) p=82;
  else if (dat<2972) p=83;
  else if (dat<3016) p=84;
  else if (dat<3060) p=85;
  else if (dat<3108) p=86;
  else if (dat<3156) p=87;
  else if (dat<3204) p=88;
  else if (dat<3252) p=89;
  else if (dat<3300) p=90;
  else if (dat<3382) p=91;
  else if (dat<3464) p=92;
  else if (dat<3546) p=93;
  else if (dat<3638) p=94;
  else if (dat<3700) p=95;
  else p=100;
  return p; 
}



void setup() 
{
  //delay(1000);
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
 
 WiFi.disconnect();
 Serial.println("Disconnecting previously connected WiFi");
 EEPROM.begin(512); //Initialasing EEPROM
 delay(10);
 Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
 
  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
    Serial.println();
    Serial.println("Waiting.");
  
    while ((WiFi.status() != WL_CONNECTED)){
      Serial.print(".");
      delay(100);
      server.handleClient();
      while(1);
    }
  }

timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);

if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Enable messages batching and retry buffer
  client.setWriteOptions(WriteOptions().writePrecision(WRITE_PRECISION).batchSize(MAX_BATCH_SIZE).bufferSize(WRITE_BUFFER_SIZE));

}

void loop() 
{
  if (iterations++ >= 360) {
    timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);
    iterations = 0;
  }
  uint8_t rxData[10];
  uint8_t rxLen = lora.Receive(rxData, 10);
  if ( rxLen > 0 )
  { 
    uint8_t note=0;
    Serial.print("Receive rxLen:");
    Serial.println(rxLen);
    for(int i=0;i< rxLen;i++) {
      Serial.print(rxData[i], HEX);
      Serial.print(" ");
    }
    lora.Send(rxData, 4, SX126x_TXMODE_SYNC);
    if (rxLen==4){
      Serial.println("new device registered");
    }
    else if (rxLen==9){
      note=rxData[8];
      Serial.println("alert!");
    }
    else{
      note=0;
    }
    Serial.println("Sensor ID:");
    char a = rxData[0];
    Serial.print(a);
    char b = rxData[1];
    Serial.print(b);
    char c = rxData[2];
    Serial.print(c);
    Serial.println(rxData[3]);
    String id = String(a)+String(b)+String(c)+String(rxData[3]);
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
      if (client.isBufferEmpty()) {
    // Report all the detected wifi networks
    time_t tnow = time(nullptr);
    Point sensorID(id);
    //sensorID.addTag("Battery", String(bat));
    //sensorID.addTag("Level", String(lev));
    //sensorID.addTag("note", String(note));
    //sensorID.addField("ID", id);
    sensorID.addField("Battery", myVar2);
    sensorID.addField("Level", converter(myVar));
    sensorID.addField("note", rxData[8]);
    sensorID.setTime(tnow);

    // Set identical time for the whole network scan
    Serial.print("Writing: ");
    Serial.println(client.pointToLineProtocol(sensorID));

      // Write point into buffer - low priority measures
    client.writePoint(sensorID);
  } else
    Serial.println("Wifi networks reporting skipped due to communication issues");

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // End of the iteration - force write of all the values into InfluxDB as single transaction
  Serial.println("Flushing data into InfluxDB");
  if (!client.flushBuffer()) {
    Serial.print("InfluxDB flush failed: ");
    Serial.println(client.getLastErrorMessage());
    Serial.print("Full buffer: ");
    Serial.println(client.isBufferFull() ? "Yes" : "No");
  }
   
  }
  delay(1);
}
