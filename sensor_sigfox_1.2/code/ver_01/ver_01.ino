#include <AS5600.h>
#include <avr/sleep.h>
#include <EEPROM.h>

#define idTag             1234 // set id
#define sleepTimerTime    10 // time sleep in second
                              // 1 hour 3600 second 

#define bataryPIN         PIN_PA4     //
#define SPGO              PIN_PB3
#define PWR               PIN_PB4
#define LED_D1            PIN_PA3


uint16_t angel, bataryVoltage;
uint8_t dataToSend[10];

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
  EEPROM.write(0, 'a');
  EEPROM.write(1, 'b');
  EEPROM.write(2, 'c');
  EEPROM.write(3, 1);
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
  dataToSend[0] = EEPROM.read(0);        
  dataToSend[1] = EEPROM.read(1);
  dataToSend[2] = EEPROM.read(2);
  dataToSend[3] = EEPROM.read(3);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
    Serial1.print("AT\r");
    delay(500);
    Serial1.print("AT$I=10\r");
    delay(1000);
    Serial1.print("AT$I=11\r");
    delay(1000);
    Serial1.print ("AT$SF=023203ff\r");
    delay(10000);
    
    Serial1.print("AT$CW=923200000, 1 , 14");
    delay(100000);
    //while(1){
      //}
  //delay (3000);
}

void loop() {
      // put your main code here, to run repeatedly:
      //loop for test begin
      //use   digitalWrite(LED_D1, 1/0) if necessary 
      //or use Serial1.println (""); 
    /*  
    digitalWrite(PWR, 1);
    delay(3000);  
    Serial1.print ("AT$SF=testmesage\r");
    delay(300);
    CheckSendData();
    for (int i=0; i<3;i++){if (verifiedSendData == 0){Serial1.print ("AT$SF=testmesage\r");delay(3000);CheckSendData();delay(3000);}}
    digitalWrite(PWR, 0);
    SleepInDownModeInterruptRTC();
    */
    
    //loop for test end
    
    //loop for TZ begin 
    /*   
      digitalWrite(PWR, 1);
      digitalWrite(LED_D1, 1);
      delay(3000); 
      GetCurrencyData();
      SendData_write();
      CheckSendDataClean();
      if (verifiedSendData == 0){
        delay(1000);
        SendData_write();
      }
      
      //for (int i=0; i<2;i++){if (verifiedSendData == 0){SendData_write();delay(3000);CheckSendDataClean();delay(3000);}}
      //digitalWrite(PWR, 0);
      digitalWrite(LED_D1, 0);
      delay(10);
      SleepInDownModeInterruptRTC();
     */  
       //loop for TZ end
       /*
        * maybe use CheckSendDataClean() it will be better for TZ 
        */
    Serial1.print("AT$CB=100,1");
    delay(1000);
    Serial1.print("AT$CB=100,0");
    delay(1000);
      
}

void SleepInDownModeInterruptRTC(){
  
  while (countRTC_CLK<sleepTimerTime){sleep_cpu();}
  countRTC_CLK=0;
  
  }


void GetCurrencyData(){

  angel= encoder.getAngle();
  //delay(100);
  bataryVoltage= analogRead(bataryPIN);
  }

void SendData_write(){
  
  dataToSend[4] = (angel >> 8) & 0xFF; 
  dataToSend[5] = angel & 0xFF; 

  dataToSend[6] = (bataryVoltage >> 8) & 0xFF; 
  dataToSend[7] = bataryVoltage & 0xFF; 

  //dataToSend[4] = (idTag >> 8) & 0xFF; 
  //dataToSend[5] = idTag & 0xFF; 

  Serial1.print ("AT$SF=");
  for(byte j=0;j<8;j++){
    Serial1.write (dataToSend[j]);
  }
  //Serial1.print ("AT$SF=\r");  `  //????
  Serial1.print ("\r");          //????^^^^
  }

void CheckSendData(){
    delay (100);
    if (Serial1.available()>0){
      verifiedSendData = false;
      char in_Serial1_data  = Serial1.read();
      //Serial1.println(in_Serial1_data);
      
      if (in_Serial1_data == 'O'){
        //Serial1.println("verifed");
        verifiedSendData = true;
        //Serial1.println(verifiedSendData);
        }  
      else{
        verifiedSendData = false;
        //Serial1.println("notVerifed");
        //Serial1.println(verifiedSendData);
      }
        
    delay (1);
    
    while(Serial1.available()>0){
      Serial1.read();
      //Serial1.println("cleaning");
      delay (1);
    }
    }
}

  
void CheckSendDataClean(){
    delay (100);
    if (Serial1.available()>0){
      verifiedSendData = false;
      char in_Serial1_data  = Serial1.read();
      if (in_Serial1_data == 'O'){  verifiedSendData = true;  }
        
    delay (5);
    
    while(Serial1.available()>0){
      Serial1.read();
      delay (5);
                               }
    }}


    
