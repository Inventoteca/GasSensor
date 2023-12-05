#include <AS5600.h>
#include <avr/sleep.h>
//#include <EEPROM.h>

//#define idTag             1234 // set id
#define sleepTimerTime    3600 // time sleep in second
                              // 1 hour 3600 second 

#define bataryPIN         PIN_PA4     //
#define SPGO              PIN_PB3
#define PWR               PIN_PB4
#define LED_D1            PIN_PA3


uint16_t dat, p=0, bataryVoltage;
//uint8_t dataToSend[8];

uint8_t txData[4];

volatile uint16_t countRTC_CLK=0;

bool verifiedSendData = false;
AS5600 encoder; 



void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */

  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 32768, resulting in 32.768kHz/32768 = 1Hz */
  | RTC_PITEN_bm;                       /* Enable PIT counter: enabled */
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
  countRTC_CLK++;
}





void setup() {
  // put your setup code here, to run once:

  RTC_init();
  analogReference(INTERNAL2V048);
  Serial1.begin(9600); // START UART
  pinMode(bataryPIN, INPUT);

  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, 1);//always off , on only wakeup
  
  pinMode(SPGO, OUTPUT);
  digitalWrite(SPGO, 1);

  pinMode(LED_D1, OUTPUT);
  digitalWrite(LED_D1, 1);
  delay(500);
  digitalWrite(LED_D1, 0);    
  delay(500);
  digitalWrite(LED_D1, 1);
  delay(500);
  digitalWrite(LED_D1, 0);    
  delay(500);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  

  //delay (3000);
}

void loop() {
      // put your main code here, to run repeatedly:
      //loop for test begin
      //use   digitalWrite(LED_D1, 1/0) if necessary 
      //or use Serial.println (""); 
    /*  
    digitalWrite(PWR, 1);
    delay(3000);  
    Serial.print ("AT$SF=testmesage\r");
    delay(300);
    CheckSendData();
    for (int i=0; i<3;i++){if (verifiedSendData == 0){Serial.print ("AT$SF=testmesage\r");delay(3000);CheckSendData();delay(3000);}}
    digitalWrite(PWR, 0);
    SleepInDownModeInterruptRTC();
    */
    
    //loop for test end
    
    //loop for TZ begin 
       
      digitalWrite(PWR, 1);
      digitalWrite(LED_D1, 1);
      delay(3000);
      Serial1.print("AT\r");
      delay(500);
      Serial1.print("AT$I=10\r");
      delay(1000);
      Serial1.print("AT$I=11\r");
      delay(1000);
      GetCurrencyData();

      SendHEXdata();
      
     // SendData_write();
     /// CheckSendDataClean();
     // if (verifiedSendData == 0){
      //  delay(1000);
      //  SendHEXdata();
        //SendData_write();
      //}
      
      //for (int i=0; i<2;i++){if (verifiedSendData == 0){SendData_write();delay(3000);CheckSendDataClean();delay(3000);}}
      digitalWrite(PWR, 0);
      digitalWrite(LED_D1, 0);
      delay(10);
      SleepInDownModeInterruptRTC();
       
       //loop for TZ end
       /*
        * maybe use CheckSendDataClean() it will be better for TZ 
        */
    
      
}

void SleepInDownModeInterruptRTC(){
  
  while (countRTC_CLK<sleepTimerTime){sleep_cpu();}
  countRTC_CLK=0;
  
  }


void GetCurrencyData(){
  p = encoder.getAngle();
/*
  dat = encoder.getAngle();
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
*/
  //p++;
  bataryVoltage= analogRead(bataryPIN);
  //Serial1.println(angel); Serial1.println(bataryVoltage);
  }
/*
void SendData_write(){
  
  dataToSend[0] = (angel >> 8) & 0xFF; 
  dataToSend[1] = angel & 0xFF; 

  dataToSend[2] = (bataryVoltage >> 8) & 0xFF; 
  dataToSend[3] = bataryVoltage & 0xFF; 

  //dataToSend[4] = (idTag >> 8) & 0xFF; 
  //dataToSend[5] = idTag & 0xFF; 

  Serial1.print ("AT$SF=");
  for(byte j=0;j<8;j++){
    Serial.write (dataToSend[j]);
  }
  //Serial.print ("AT$SF=\r");  `  //????
  Serial1.print ("\r");          //????^^^^
  }
*/
void CheckSendData(){
    delay (100);
    if (Serial1.available()>0){
      verifiedSendData = false;
      char in_serial_data  = Serial1.read();
      //Serial.println(in_serial_data);
      
      if (in_serial_data == 'O'){
        //Serial.println("verifed");
        verifiedSendData = true;
        //Serial.println(verifiedSendData);
        }  
      else{
        verifiedSendData = false;
        //Serial.println("notVerifed");
        //Serial.println(verifiedSendData);
      }
        
    delay (1);
    
    while(Serial1.available()>0){
      Serial1.read();
      //Serial.println("cleaning");
      delay (1);
    }
    }
}

  
void CheckSendDataClean(){
    delay (100);
    if (Serial1.available()>0){
      verifiedSendData = false;
      char in_serial_data  = Serial1.read();
      if (in_serial_data == 'O'){  verifiedSendData = true;  }
        
    delay (5);
    
    while(Serial1.available()>0){
      Serial1.read();
      delay (5);
                               }
    }}

void SendHEXdata(){
  
  txData[0] = (p >> 8) & 0xFF; 
  txData[1] = p & 0xFF;  
  txData[2] = (bataryVoltage >> 8) & 0xFF; 
  txData[3] = bataryVoltage & 0xFF; 

  
  Serial1.print ("AT$SF=");
  if (txData[0] < 0x10) Serial1.print("0");
  Serial1.print(txData[0], HEX);
  if (txData[1] < 0x10) Serial1.print("0");
  Serial1.print(txData[1], HEX);
  if (txData[2] < 0x10) Serial1.print("0");
  Serial1.print(txData[2], HEX);
  if (txData[3] < 0x10) Serial1.print("0");
  Serial1.print(txData[3], HEX); 
  Serial1.print("\r");
  
  delay(5000);
  /*
  txData[0] = (angel >> 8) & 0xFF; 
  txData[1] = angel & 0xFF;  
   txData[2] = (bataryVoltage >> 8) & 0xFF; 
  txData[3] = bataryVoltage & 0xFF; 

  
  String s2(txData[0], HEX);
   if (s2[1] =='\0') {s2= '0'+s2;}
  String s3(txData[1], HEX);
    if (s3[1] =='\0') {s3= '0'+s3;}
    String s4(txData[2], HEX);
  if (s4[1] =='\0') {s4= '0'+s4;}
  String s5(txData[3], HEX);
    if (s5[1] =='\0') {s5= '0'+s5;}
    
  s2 += s3;
  s2 += s4;
  s2 += s5;
  s2= "AT$SF="+ s2;
  s2+= "\r";
  Serial1.print (s2);
  delay(500);
  */
  }

    
