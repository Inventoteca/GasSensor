#include <SPI.h>
#include <Ra01S.h>
#include <AS5600.h>
#include <EEPROM.h>
#include <avr/sleep.h>
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
//#define PERIOD      10 
#define PERIOD      900 //15 min
#define ALA         200
#define LED         PIN_PB5
#define PWR         PIN_PB4
#define BAT         PIN_PC1

#define DIO1        PIN_PA7
#define DIO2        PIN_PA5

//#define SDIR        PIN_PB3
#define SPGO        PIN_PC3
#define SOUT        PIN_PC0

//#if 1
/*
 * for ATmega328/2560
 * VCC    3V3/3V3
 * GND    GND/GND
 * SCK    13/52
 * MISO   12/50
 * MOSI   11/51
 * NSS     5/5
 * RST     6/6
 * BUSY    7/7
 */
uint16_t ang_old=0, ang=0, bat=0;
uint8_t txData[9], rxData[4];
uint8_t rxLen=0, hops=0;
volatile uint16_t count=0;
boolean stat=0, note=0;
AS5600 encoder;
SX126x  lora(PIN_PC2,               //Port-Pin Output: SPI select
             PIN_PA6,               //Port-Pin Output: Reset 
             PIN_PB3                //Port-Pin Input:  Busy
             );

//#endif // ATmega328/2560

void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */

  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 16384, resulting in 32.768kHz/16384 = 2Hz */
  | RTC_PITEN_bm;                       /* Enable PIT counter: enabled */
}

ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
  count++;
}


void setup() 
{
  RTC_init(); 
  EEPROM.write(0, 'a');
  EEPROM.write(1, 'b');
  EEPROM.write(2, 'c');
  EEPROM.write(3, 4);
  pinMode(LED, OUTPUT);
  pinMode(PWR, OUTPUT);
  //pinMode(SDIR, OUTPUT);
  pinMode(SPGO, OUTPUT);
  pinMode(BAT, INPUT);
  pinMode(DIO1, INPUT);
  pinMode(DIO2, INPUT);
  pinMode(SOUT, INPUT);
  analogReference(INTERNAL2V048);
  txData[0] = EEPROM.read(0);        
  txData[1] = EEPROM.read(1);
  txData[2] = EEPROM.read(2);
  txData[3] = EEPROM.read(3);// ID, 4bytes
  digitalWrite(PWR, 1);//always on
  //digitalWrite(SDIR, 1);
  digitalWrite(SPGO, 1);
  SPI.begin();
  digitalWrite(LED, 1);
  delay(500);
  digitalWrite(LED, 0);
  delay(500);
  digitalWrite(LED, 1);
  delay(500);
  digitalWrite(LED, 0);
  delay(500);

  //lora.DebugPrint(true);

  int16_t ret = lora.begin(RF_FREQUENCY,              //frequency in Hz
                           TX_OUTPUT_POWER);          //tx power in dBm
  if (ret != ERR_NONE) while(1) {
    delay(1);
    digitalWrite(LED, 1);
    delay(100);
    digitalWrite(LED, 0);
    delay(100);
    }


  lora.LoRaConfig(LORA_SPREADING_FACTOR, 
                  LORA_BANDWIDTH, 
                  LORA_CODINGRATE, 
                  LORA_PREAMBLE_LENGTH, 
                  LORA_PAYLOADLENGTH, 
                  true,               //crcOn  
                  false);             //invertIrq
reg:
  if (lora.Send(txData, 4, SX126x_TXMODE_SYNC)) {
    //digitalWrite(LED, 1);
    //delay(1000);
    //digitalWrite(LED, 0);
     while(rxLen<=0){
      rxLen = lora.Receive(rxData, 4);
     }
    if((rxData[0]==txData[0])&&(rxData[1]==txData[1])&&(rxData[2]==txData[2])&&(rxData[3]==txData[3])){
      digitalWrite(LED, 1);
      delay(1000);
      digitalWrite(LED, 0);
     }
     else goto reg;
     
  } else {
    while(1){
      digitalWrite(LED, 1);
      delay(300);
      digitalWrite(LED, 0);
      delay(300);
    }
  }
  /*
  while(rxLen<=0){
    rxLen = lora.Receive(rxData, 4);
  }
  if((rxData[0]==txData[0])&&(rxData[1]==txData[1])&&(rxData[2]==txData[2])&&(rxData[3]==txData[3])){
    digitalWrite(LED, 1);
    delay(1000);
    digitalWrite(LED, 0);
  }
  else goto reg;*/
  ang_old = encoder.getAngle();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  /* Set sleep mode to POWER DOWN mode */
  digitalWrite(PWR, 0);
  delay(5000);
}


void loop() 
{
 slp:
  sleep_cpu();
  if (count<PERIOD) goto slp;
  uint8_t txData[10];
  //sprintf((char *)txData, "Hello World %lu", millis());
  //ADC0.CTRLA |= ~ADC_ENABLE_bm;
  digitalWrite(PWR, 1);
  txData[0] = EEPROM.read(0);        
  txData[1] = EEPROM.read(1);
  txData[2] = EEPROM.read(2);
  txData[3] = EEPROM.read(3);// ID, 4bytes
  ang = encoder.getAngle();
  if (ang >4000){
    //ADC0.CTRLA &= ~ADC_ENABLE_bm;
    digitalWrite(PWR, 0);
    goto slp;
  }
  bat=analogRead(BAT);
  txData[5] = ang & 0xFF;         // extract least significant byte
  txData[4] = (ang >> 8) & 0xFF;  // extract most significant byte
  txData[7] = bat & 0xFF;         // extract least significant byte
  txData[6] = (bat >> 8) & 0xFF;  // extract most significant byte
  //uint8_t len = strlen((char *)txData);
  if (ang_old < ang){
    if ((ang - ang_old)>ALA){
      txData[8]=2;
      note=1;
    }
    else {
      txData[8]=0;
      note=0;
    }
  }
  else if (ang_old > ang){
    if ((ang_old - ang)>ALA){
      txData[8]=1;
      note=1;
    }
    else {
      txData[8]=0;
      note=0;
    }
  }
  else {
    txData[8]=0;
    note=0;
  }
  ang_old = ang;
  // Wait for transmission to complete
sending:  
  if (note==1){
    stat=lora.Send(txData, 9, SX126x_TXMODE_SYNC);
  }
  else{
    stat=lora.Send(txData, 8, SX126x_TXMODE_SYNC);
  }
  if(stat==0){//problem
    digitalWrite(LED, 1);
    delay(5000);
    digitalWrite(LED, 0);
    //delay(300);
    //digitalWrite(LED, 1);
    //delay(300);
    //digitalWrite(LED, 0);
  }
  else{
    hops++;
    unsigned long start_time=millis();
    while((rxLen<=0)&&((millis()-start_time)<3000)){
      rxLen = lora.Receive(rxData, 4); 
      if((rxData[0]==txData[0])&&(rxData[1]==txData[1])&&(rxData[2]==txData[2])&&(rxData[3]==txData[3])){
        break;
      }
      else rxLen=-1;
    }
    if(((millis()-start_time)>3000)&&(hops<3)){
      goto sending;
    }
    else(hops=0);
  }
  digitalWrite(PWR, 0);
  //ADC0.CTRLA &= ~ADC_ENABLE_bm;
  digitalWrite(LED, 1);
  delay(100);
  digitalWrite(LED, 0);
  count=0;
  
  // Do not wait for the transmission to be completed
  //lora.Send(txData, len, SX126x_TXMODE_ASYNC );

  //delay(2000);
}
