// Hello LoRa - ABP TTN Packet Sender (Multi-Channel)
//
// This sample code send some payload (temp,dh11,volatage, and display information on a SSD1306)
// The program also put the arduino into sleep mode ... We used a feather which used only 0.022A - 0.010A
//
// Note: The Serial port stops working when in sleep mode .
// 
// markus@vankempen.org
//
/************************** Configuration ***********************************/
#define VERSION "2021-02-25-13:33-V9"
#define SERIALON 1

#include <TinyLoRa.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <CayenneLPP.h>
CayenneLPP lpp(51);
#define VBATPIN A7
#include <RTCZero.h>
#include <Adafruit_SleepyDog.h>
#include "DHT.h"

//Feather M
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
  volatile unsigned long last_millis;
  volatile int button_A_state = HIGH;
  long debouncing_time = 250;

// How many times data transfer should occur, in seconds
const unsigned int sendInterval = 10 ;  ///<<<<<<<<<<<< every 10 min



extern "C" char *sbrk(int i); //  Used by FreeRAm Function

/* Create an rtc object */
RTCZero rtc;
int AlarmTime;
/////////////// Global Objects ////////////////////

#define SampleIntSec 15 // RTC - Sample interval in seconds
#define SamplesPerCycle 60  // Number of samples to buffer before uSD card flush is called

const int SampleIntSeconds = 10;   //Simple Delay used for testing, ms i.e. 1000 = 1 sec

/* Change these values to set the current initial time */
const byte hours = 18;
const byte minutes = 50;
const byte seconds = 0;
/* Change these values to set the current initial date */
const byte day = 29;
const byte month = 12;
const byte year = 15;   
float savetemp =0;


float measuredvbat;   // Variable for battery voltage
int NextAlarmSec; // Variable to hold next alarm time in seconds
unsigned int CurrentCycleCount;  // Num of smaples in current cycle, before uSD flush call

// Pin the DHT22 is connected to
#define DHTPIN 10
DHT dht(DHTPIN, DHT11);



Adafruit_SSD1306 display = Adafruit_SSD1306();

// Visit your thethingsnetwork.org device console
// to create an account, or if you need your session keys.

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x28, 0x6B, 0xFB, 0xF8, 0x5F, 0xA2, 0xB3, 0x64, 0x10, 0xAB, 0xE0, 0x63, 0xCF, 0x34, 0x8E, 0x75 };
//286BFBF85FA2B36410ABE063CF348E75

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x60, 0xA4, 0x94, 0x20, 0x56, 0x8C, 0x50, 0xF2, 0xFB, 0x6E, 0x8A, 0xF7, 0x61, 0xDC, 0x94, 0xAF };
//60A49420568C50F2FB6E8AF761DC94AF

// Device Address (MSB)
uint8_t DevAddr[4] = { 0x26, 0x01, 0x36, 0x2F };
//26 01 36 2F
/************************** Example Begins Here ***********************************/
// Data Packet to Send to TTN
unsigned char loraData[11] = {"hello LoRa"};

// Pinout for Adafruit Feather 32u4 LoRa
//TinyLoRa lora = TinyLoRa(7, 8, 4);

// Pinout for Adafruit Feather M0 LoRa
TinyLoRa lora = TinyLoRa(3, 8, 4);

/************************** SETUP ***********************************/
void setup()
{

  Serial.begin(9600);
  //while (! Serial);
  delay(1000);

    // Initialize pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(BUTTON_A, INPUT_PULLUP);
  
  
  // this is for sleep mode 
  rtc.begin();    // Start the RTC in 24hr mode
  rtc.setTime(hours, minutes, seconds);   // Set the time
  rtc.setDate(day, month, year);    // Set the date

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  delay(1000);

// Initialize DHT
  dht.begin();
  delay(500);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
   delay(500);
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
   myprintln("readTemperature() ="+String(t));
   delay(500);
   myprintln("readHumidity() ="+String(h));
 
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    myprintln("Failed to read from DHT sensor!");
     say( "","Failed to read", "from DHT sensor!", "We Try again ","");
    delay(2000);   // Stops freakout when DHT bad
// Initialize DHT
  dht.begin();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    myprintln("Failed to read from DHT sensor! 2nd");
     say( "","Failed to read", "from DHT sensor!", "2nd","");
    delay(2000);   // Stops freakout when DHT bad
  }//2nd
  }//1st

  myprintln(">>>>> Starting feather Lora Hello World");
  say( "Starting Lora","Version:", VERSION, "",getHexString(AppSkey));
  delay(2000);  

  Serial.print("DevAddr:");
 // Serial.println((char)DevAddr);
  for(int i=0; i<sizeof(DevAddr); i++)
  printHex(DevAddr[i]);
  Serial.println();

  Serial.print("NwkSkey:"); 
  for(int i=0; i<sizeof(NwkSkey); i++)
  printHex(NwkSkey[i]);
  Serial.println();

    Serial.print("AppSkey:");
  for(int i=0; i<sizeof(AppSkey); i++)
  printHex(AppSkey[i]);
  Serial.println();

  Serial.println(getHexString(AppSkey));

  
  // Initialize LoRa
  myprintln("Starting LoRa...");
  // define multi-channel sending
  lora.setChannel(MULTI);
  // set datarate
  lora.setDatarate(SF7BW125);
  if(!lora.begin())
  {
    myprintln("Failed");
    myprintln("Check your radio");
      say( "Error","Lora start", "Failed", "We wait 5sec","");
        delay(3000);
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        delay(10000);
        NVIC_SystemReset();
  }

  // Optional set transmit power. If not set default is +17 dBm.
  // Valid options are: -80, 1 to 17, 20 (dBm).
  // For safe operation in 20dBm: your antenna must be 3:1 VWSR or better
  // and respect the 1% duty cycle.

  // lora.setPower(17);

     say( "LoRa is Ok!","", "Starting Loop", "","");
     delay(5000);

     
  attachInterrupt(digitalPinToInterrupt(BUTTON_A), ButtonwakeUp, FALLING);

}
/*
 * LOOP
 */

String getHexString(uint8_t* num)
{//  
  String s="";
  for(int i=0; i<sizeof(num); i++)
  {
  char hexCar[2];
  sprintf(hexCar, "%02X", num[i]);
  s=s+hexCar;
  }
  return(s);
}  

void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

void loop()
{



 
  CurrentCycleCount += 1;     //  loop counter
  
  Serial.println("Sending LoRa Data...");

  //llp
          /*
           * 
        uint8_t addDigitalInput(uint8_t channel, uint8_t value);
        uint8_t addDigitalOutput(uint8_t channel, uint8_t value);

        uint8_t addAnalogInput(uint8_t channel, float value);
        uint8_t addAnalogOutput(uint8_t channel, float value);

        uint8_t addLuminosity(uint8_t channel, uint16_t lux);
        uint8_t addPresence(uint8_t channel, uint8_t value);
        uint8_t addTemperature(uint8_t channel, float celsius);
        uint8_t addRelativeHumidity(uint8_t channel, float rh);
        uint8_t addAccelerometer(uint8_t channel, float x, float y, float z);
        uint8_t addBarometricPressure(uint8_t channel, float hpa);
        uint8_t addGyrometer(uint8_t channel, float x, float y, float z);
        uint8_t addGPS(uint8_t channel, float latitude, float longitude, float meters);

           */
    if(rtc.getHours() == 0 ) //reset every day 
       NVIC_SystemReset();
 
   if( (savetemp != getTemperature()) || (rtc.getMinutes() == 0 )) // mini every hour 
   //|| ((CurrentCycleCount % sendInterval) == 0) ) // if temperature has changed send data or send every 100(sendInterval) message 
  //if(true)
  {
    lpp.reset();
    lpp.addTemperature(1, getTemperature());
    lpp.addBarometricPressure(1, getHumidity());
    lpp.addAnalogInput(1, lora.frameCounter);
    lpp.addAnalogInput(2, CurrentCycleCount);
    lpp.addAnalogOutput(1,BatteryVoltage());
    lpp.addAnalogOutput(2,(CurrentCycleCount % sendInterval) );
    SerialOutput(); // prints all data
 
     savetemp =getTemperature();     

    if(CurrentCycleCount > 1440)   // reset    CurrentCycleCount to zero
    {
      CurrentCycleCount=0;
    }
    
// LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
// send the 6 bytes payload to LoRaWAN port 7
//LMIC_setTxData2(7, lpp.getBuffer(), lpp.getSize(), 0);
             
 // say( "Sending LoRa Data:","", "", "","");
  Serial.print("Sending: ");  
  lora.sendData(lpp.getBuffer(), lpp.getSize(), lora.frameCounter);
    for(int i=0; i<sizeof(lpp.getBuffer()); i++)
  printHex(lpp.getBuffer()[i]);
  Serial.println();
  
  // Optionally set the Frame Port (1 to 255)
  // uint8_t framePort = 1;
  // lora.sendData(loraData, sizeof(loraData), lora.frameCounter, framePort)

  lora.frameCounter++;

  say( "Sensor Info Frame:"+String( lora.frameCounter),"Temp:"+String(getTemperature()),"Humi:"+String(getHumidity()), "Volt:"+String(BatteryVoltage())+" Int: "+String(sendInterval),"Frame = "+String( lora.frameCounter));
  
  delay(5000);  

  lora.sendData(loraData, sizeof(loraData), lora.frameCounter);
  say( "Hello Frame:"+String( lora.frameCounter),"","","String:hello LoRa","");
    Serial.print("Sending2: ");    Serial.println("hello LoRa");
    
   lora.frameCounter++;
//  myprintln("Putting lora to sleep!");
//  lora.sleep();
  //lora.end()
   delay(5000); 
  // blink LED to indicate packet sent
  blink(LED_BUILTIN,1);

  //Time String
  String mytime=String(rtc.getMinutes());
  
  if(rtc.getMinutes() < 10)
       mytime="0"+rtc.getMinutes();

  if(rtc.getSeconds() < 10)
      mytime=mytime+":0"+rtc.getSeconds();
  else
      mytime=mytime+":"+rtc.getSeconds();
  
  //say( "Time (mm/ss)= "+mytime,"Data Send", "Next msg in "+String(sendInterval)," Minute","");
  delay(5000);

   }else{
      myprintln("No temperature Data change");
     say( "Sensor Info Loop:"+String( CurrentCycleCount),"Temp = "+String(getTemperature()),"Humi = "+String(getHumidity()), ">>No CHange","");
    delay(3000);
          
  }
  savetemp =getTemperature();  

    
 //AlarmTime = rtc.getSeconds()+sendInterval; // Adds 10 seconds to alarm time
 AlarmTime = rtc.getMinutes()+sendInterval;

 if (AlarmTime > 59)
    AlarmTime=sendInterval;
    
// AlarmTime = AlarmTime % 60; // checks for roll over 60 seconds and corrects

  //rtc.setAlarmSeconds(    ); // Wakes at next alarm time
 // rtc.enableAlarm(rtc.MATCH_SS); // Match seconds only

  rtc.setAlarmMinutes(AlarmTime); // Set minutes alarm
  rtc.setAlarmSeconds(0);                          // Set seconds alarm = 0
  rtc.enableAlarm(rtc.MATCH_MMSS);
  
  myprintln("Putting Arduino to sleep!");
  myprint("for ");
  myprintln(String(AlarmTime));
  
  say( "","Standby! Mode", "Next Alarm Time "+String(AlarmTime),"Minutes","");
  delay(5000);

  display.ssd1306_command(SSD1306_DISPLAYOFF);
  delay(5000);
   rtc.attachInterrupt(alarmMatch);

  myprintln("-------------------------");
  myprintln("");
  rtc.standbyMode();    // Sleep until next alarm match

  
//Serial.attach();
//  Serial.begin(9600);

// a different way of puttung Arduino to sleep 
 //int sleepMS = Watchdog.sleep(600000);
//    USBDevice.attach();
//     Serial.begin(9600);

       
} //end loop

void ButtonwakeUp() {

if((millis() - last_millis) > debouncing_time) {
  Serial.println("Wake Button A S");
  say( "Wake Button A","", "", "","");
    last_millis = millis();
    button_A_state = !button_A_state;
    delay(200);
    digitalWrite(BUTTON_A, button_A_state);  
  }
}


/************************** functions Example Begins Here ***********************************/


void  myprintln(String s)
{
#ifdef SERIALON
  Serial.println(s);
#endif   
}

void myprint(String s)
{
#ifdef SERIALON
  Serial.print(s);
#endif   
}

// print to oled
void say(String s, String t, String u, String v,String w) {

  
display.ssd1306_command(SSD1306_SETCONTRAST);
display.ssd1306_command(0);
display.ssd1306_command(SSD1306_DISPLAYOFF); // To switch display off
display.ssd1306_command(SSD1306_DISPLAYON); // To switch display back on

  display.dim(true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println(s);
  display.println(t);
  display.println(u);
  display.println(v);
   display.setCursor(30,30);
  display.print(w);
#ifdef SERIALON
if(s !="")
  Serial.println(s);
if(t !="")  
  Serial.println(t);
if(u !="")  
  Serial.println(u);
if(v !="")  
  Serial.println(v);
if(w !="")  
  Serial.println(w);
#endif  
  
  display.display();
}
void(* resetFunc) (void) = 0; //declare reset function @ address 0



// Debbugging output of time/date and battery voltage
void SerialOutput() {
   Serial.println(">>>> DEBUG");
  Serial.print(CurrentCycleCount);
  Serial.print(":Mem:");
  Serial.print(freeram ());
  Serial.print(" / Date:");
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear()+2000);
  Serial.print(" ");
  Serial.print(rtc.getHours());
  Serial.print(":");
  if(rtc.getMinutes() < 10)
    Serial.print('0');      // Trick to add leading zero for formatting
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  if(rtc.getSeconds() < 10)
    Serial.print('0');      // Trick to add leading zero for formatting
  Serial.print(rtc.getSeconds());
  Serial.print(",Voltage:");
  Serial.print(BatteryVoltage ());   // Print battery voltage  
  Serial.print(",LoRaFrame:");
  Serial.print(lora.frameCounter);   
  Serial.print(",Temp:");
  Serial.print(getTemperature());   // Print battery voltage  
    Serial.print(",Humi:");
  Serial.println(getHumidity());   // Print battery voltage    
      Serial.println("<<<< DEBUG");
}
// blink out an error code, Red on pin #13 or Green on pin #8
void blink(uint8_t LED, uint8_t flashes) {
  uint8_t i;
  for (i=0; i<flashes; i++) {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(200);
  }
}
//

float getTemperature()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    myprintln("Failed to read from DHT(t) sensor!");
    say( "","Failed to read", "from DHT(t) sensor!", "","");
    delay(2000);   // Stops freakout when DHT bad
    return(0);
  }
     return(t);
}

float getHumidity()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) ) {
    myprintln("Failed to read from DHT(h) sensor!");
    say( "","Failed to read", "from DHT(h) sensor!", "","");
    delay(2000);   // Stops freakout when DHT bad
    return(0);
  }
   return(h);
}
    
// Measure battery voltage using divider on Feather M0 - Only works on Feathers !!
float BatteryVoltage () {
  measuredvbat = analogRead(VBATPIN);   //Measure the battery voltage at pin A7
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}

void alarmMatch() // Do something when interrupt called
{
  
}

int freeram () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}
