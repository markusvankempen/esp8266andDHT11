//
// Simple WebServer with mqtt messaging
//
// mvk@ca.ibm.com - for SNP/ P-Tech
//
// This programs will start a websever which is connected to you wifi once you adjust the SSID and password please change all parameters - see CHANGE HERE
// the code will also send a mqtt message a mqtt broker .... 
// 
//

// based on https://randomnerdtutorials.com/esp8266-web-server/
// switch on /off LED ... LED GPIO16 or GPIO03

#define VERSION "161220202-1323"

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <PubSubClient.h> //<<<< lib needs to be installed 

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TIME Server NTP 
#include <WiFiUdp.h>
#include <NTPClient.h>
WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
//NTPClient timeClient(ntpUDP);
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  // EST = GMT -5
  
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -18000,6000);
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
// Variables to save date and time
//https://gndtovcc.home.blog/2020/04/12/esp8266-nodemcu-ntp-client-server-get-date-and-time-arduino-ide/
String formattedDate;
String dayStamp;
String timeStamp;
///////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TIME END


// to be able to read voltage 
ADC_MODE(ADC_VCC); //vcc read

/*
////>>>>>>>>> CHANGE HERE
*/
// Replace with YOUR network credentials  
const char* ssid     = "vanishinglake";    //>>>>>>>>> CHANGE HERE
const char* password = "2Fast4You";        //>>>>>>>>> CHANGE HERE
String DEVICEID   =    "mvk01";            //>>>>>>>>> CHANGE HERE
#define mqtt_server    "52.117.240.201"    //>>>>>>>>> CHANGE HERE - should preset for P-Tech
 
//### YOUR LOCATION 
// find ur LAT LON  use https://www.latlong.net/ and ur cityname like Brantford,canada  for Branford =43.139410,-80.263650

String  LATITUDE  =   "43.6711581"   ;         //>>>>>>>>> CHANGE HERE
String  LONGITUDE =   "-79.4129989"    ;       //>>>>>>>>> CHANGE HERE


#define DHTPIN 4 // what digital pin we're connected to NodeMCU (D2)

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

String mqttmsg = "{\"hello\":\"hello\"}";
  
// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String LEDState = "on ";

// Assign output variables to GPIO pins
const int LED = 16;

// Current time
unsigned long currentTime1 = millis();
// Previous time
unsigned long previousTime1 = millis(); 

const long mqttSchedule = 10000; // send mqtt message every 10sec

unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

//dht vars
float prevTemp=0;
float h =0;
float t =0;
uint8_t mac[6];
long msgcounter=0;
float myvol=0;
//
// Setup 
//
void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(LED, OUTPUT);
  // Set outputs to LOW
  digitalWrite(LED, LOW);

  Serial.print("VERSION = ");
  Serial.println(VERSION);
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
   WiFi.macAddress(mac);
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  delay(500);
  dht.begin();
  delay(500);

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  Serial.print("readHumidity = ");
  Serial.println(h);

  Serial.print("readTemperature = ");
  Serial.println(t);

  //  Batt = ESP.getVcc();
  Serial.print("Read internal Voltage = ");
  Serial.println( ESP.getVcc());
  Serial.println("Start WebServer");
   
  //  Batt = ESP.getVcc();
  Serial.println("Read internal Voltage = "+ESP.getVcc()); // Exerise display the voltage on the website
  mqttclient.setServer(mqtt_server, 1883);
  timeClient.begin();
  timeClient.update();
//long timeOffset for EST 
  Serial.println(timeClient.getFormattedTime());
 Serial.println(timeClient.getEpochTime());
 
   unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
   Serial.print("Week Day: ");
  Serial.println(timeClient.getDay());    
 
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
 
  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);
 
  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

   int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);
 
  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

 
    
    
}
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

//
// Create MQTT connection 
//

void reconnect() {
  // Loop until we're reconnected
  Serial.println("MQTT connected = ");
   Serial.println(mqttclient.connected());
  while (!mqttclient.connected()) {
        Serial.print("Attempting MQTT connection...");
    
          // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;  
      clientName += "esp8266-";
     // uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      clientName += "-";
      clientName += String(micros() & 0xff, 16);
      Serial.print("Connecting to ");
      Serial.print(mqtt_server);
      Serial.print(" as ");
      Serial.println(clientName);


    // Attempt to connect
    // If you do not want to use a username and password, change next line to
  if (mqttclient.connect((char*) clientName.c_str())) {
    //if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password)) {
      Serial.println("connected");
         Serial.println("Sending hello message");
         mqttmsg = "{\"hello\":\""+DEVICEID+"\"}"; 
          mqttclient.publish("hello", mqttmsg.c_str(), true);
          
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//
// loop 
//
void loop(){
    WiFiClient client = server.available();   // Listen for incoming clients
    currentTime1 = millis();
    //previousTime1 = currentTime1;
    
  if((currentTime1 - previousTime1) > mqttSchedule ) // send all 5sec
  {

        h = dht.readHumidity();
      // Read temperature as Celsius (the default)
       t = dht.readTemperature();

      // Excerise ... only send message if temperature or Humidity is changing
  
      if (!mqttclient.connected()) 
        reconnect();
      
            /// SEND MQTT message
                  if (isnan(h) || isnan(t) ) {
                    Serial.println("Failed to read from DHT sensor!");
               //return;
               h=0;
               t=0;
                }
            //WiFi.localIP() // IP address 
            //macToStr(mac);  // Mac Address
            //timeClient.getEpochTime() // timestamp
            String mymac=macToStr(mac);
            String myip=WiFi.localIP().toString().c_str();
            long mydts=timeClient.getEpochTime();
            float myvol = ESP.getVcc()/1000;
            msgcounter++;
            //###EXCERISE add a message counter (msgcnt) to the json message like \"msgcounter\" : "+String(msgcounter)+ "
            mqttmsg = "{\"mac\" :  \""+mymac+"\",\"ip\" : \""+myip+"\",\"voltage\" :  "+myvol+",\"dts\" : "+mydts+",\"deviceid\" : \""+ DEVICEID+"\", \"humidity\" : " +String(h)+" ,\"temperature\" : "+String(t)+" ,\"latitude\" :"+String(LATITUDE)+",\"logitude\" : "+String(LONGITUDE)+"}";
            
            
           Serial.println("Sending Sensor Data");
           Serial.println(mqttmsg.c_str()); 
           mqttclient.publish("sensor", mqttmsg.c_str(), true);
        
          previousTime1 = millis(); 
        
    }

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;

    
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,


      prevTemp=t;
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      t = dht.readTemperature();
      
      Serial.print("Time = ");
      Serial.println(timeClient.getFormattedTime());
 
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) ) {
      Serial.println("Failed to read from DHT sensor!");
      //return;
       h = 0;
        t = 0;
      }
        
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /led/on") >= 0) {
              Serial.println("LED 5 on");
              LEDState = "on";
              digitalWrite(LED, HIGH);
            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("LED off");
              LEDState = "off";
              digitalWrite(LED, LOW);
            } 
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
    
 
            String  ptr = "";
            ptr += "<style>html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #333333;}\n";
            ptr += "body{margin-top: 50px;}\n";
            ptr += "h1 {margin: 50px auto 30px;}\n";
            ptr += ".side-by-side{display: inline-block;vertical-align: middle;position: relative;}\n";
            ptr += ".humidity-icon{background-color: #3498db;width: 30px;height: 30px;border-radius: 50%;line-height: 36px;}\n";
            ptr += ".humidity-text{font-weight: 600;padding-left: 30px;font-size: 30px;width: 200px;text-align: left;}\n";
            ptr += ".humidity{font-weight: 300;font-size: 30px;color: #3498db;}\n";
            ptr += ".temperature-icon{background-color: #f39c12;width: 30px;height: 30px;border-radius: 50%;line-height: 40px;}\n";
            ptr += ".temperature-text{font-weight: 600;padding-left: 30px;font-size: 30px;width: 200px;text-align: left;}\n";
            ptr += ".temperature{font-weight: 300;font-size: 30px;color: #f39c12;}\n";
            ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;right: -20px;top: 15px;}\n";
            ptr += ".data{padding: 10px;}\n";
            ptr += ".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\n";
            ptr += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\n";
            ptr += ".button2 {background-color: #77878A;}\n";
            ptr += "</style></head>\n";
  client.println(ptr);
  
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 16 
            client.println("<p>GPIO16 LED - State " + LEDState + "</p>");
            // If the output5State is off, it displays the ON button       
            if (LEDState=="off") {
              client.println("<p><a href=\"/led/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            client.println("<div class=\"temperature\">"+timeClient.getFormattedTime() +"</div><br>");
            
            client.println("<p>prevTemp     = " + (String)prevTemp + "</p>");
            client.println("<p>msgcounter     = " + (String)msgcounter + "</p>");
            
            if (prevTemp == t)
              client.println("<div class=\"temperature\">NO Temperature Change</div><br>");
            else
              client.println("<div class=\"temperature\">Temperature Changed from " + (String)prevTemp + "</div><br>");

            prevTemp = t;
            client.println("<div class=\"side-by-side temperature-text\">Temperature</div><div class=\"side-by-side temperature\">" + (String)t + " C</div><br>");
            client.println("<div class=\"side-by-side humidity-text\">Humidity</div><div class=\"side-by-side humidity\">" + (String)h + " %</div>\n");
            //###>>EXERISE - display the volatge
            myvol = ESP.getVcc();
            myvol = myvol / 1000;

            client.println("<br><div class=\"side-by-side humidity-text\">Voltage</div><div class=\"side-by-side humidity\">" + String(myvol) + " V</div>\n");
            /*
            client.println("<p>Temperature  = "+(String)t+"</p>");
            client.println("<p>Humidity     = "+(String)h+"</p>");
            client.println("<p>getEpochTime = "+String(timeClient.getEpochTime())+"</p>");
            client.println("<p>Time         = "+String(timeClient.getFormattedTime())+"</p>");
            client.println("<p>msgcounter   = "+String(msgcounter)+"</p>");
            */

            client.println("<p><a href=\"/refresh\"><button class=\"button button2\">Refresh</button></a></p>");

            client.println("<hr><p>mqttmsg = "+mqttmsg+"</p>");
                    
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();    
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
