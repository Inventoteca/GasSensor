#include <AS5600.h>
#include <avr/sleep.h>
//#include <EEPROM.h>

#define idTag             1234 // set id
#define sleepTimerTime    150 // time sleep in second
                              // 1 hour 3600 second 

#define bataryPIN         PIN_PA4     //
#define SPGO              PIN_PB3
#define PWR               PIN_PB4
#define LED_D1            PIN_PA3


uint16_t angel, bataryVoltage;
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

  angel= encoder.getAngle();
  //delay(100);
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
  
  txData[0] = (angel >> 8) & 0xFF; 
  txData[1] = angel & 0xFF;  
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

    
