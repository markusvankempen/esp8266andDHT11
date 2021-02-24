//
// Simple WebServer with mqtt messaging
//
// mvk@ca.ibm.com - for SNP/ P-Tech / 22-Dec-2020
//
// This programs will start a websever which is connected to you wifi once you adjust the SSID and password please change all parameters - see CHANGE HERE
// the code will also send a mqtt message a mqtt broker ....
//
// Code Freatures:
// Page auto refesh
// deepsleep with pin reset
// send mqtt message only if temperature has changed
// ntp client integration
// ToDo:
// - 

// based on https://randomnerdtutorials.com/esp8266-web-server/
// switch on /off LED ... LED GPIO16 or GPIO03

#define VERSION "20210224-1323"

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

NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -18000, 6000);
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
// Variables to save date and time
//https://gndtovcc.home.blog/2020/04/12/esp8266-nodemcu-ntp-client-server-get-date-and-time-arduino-ide/
String formattedDate;
String dayStamp;
String timeStamp;
//Greenwich Mean Time (GMT) is often interchanged or confused with Coordinated Universal Time (UTC).
///////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< TIME END


// to be able to read voltage
ADC_MODE(ADC_VCC); //vcc read

/*
  ////>>>>>>>>> CHANGE HERE
*/
// Replace with YOUR network credentials
const char* ssid     = "1Aoffice";    //>>>>>>>>> CHANGE HERE
const char* password = "2Fast4You!";        //>>>>>>>>> CHANGE HERE
String DEVICEID   =    "mvk01";            //>>>>>>>>> CHANGE HERE
#define mqtt_server    "52.117.240.201"    //>>>>>>>>> CHANGE HERE - should preset for P-Tech

//### YOUR LOCATION
// find ur LAT LON  use https://www.latlong.net/ and ur cityname like Brantford,canada  for Branford =43.139410,-80.263650

String  LATITUDE  =   "43.6711"   ;         //>>>>>>>>> CHANGE HERE
String  LONGITUDE =   "-79.4129"    ;       //>>>>>>>>> CHANGE HERE


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

const long mqttSchedule = 30000; // send mqtt message every 30sec

unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

//dht vars
float prevTemp = 0;
float h = 0;
float t = 0;
uint8_t mac[6];
long msgcounter = 0;
float myvol = 0;
int D1=5; ///GPIO5=D1 

//
// Setup
//
void setup() {
  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);  //D1 -> RESET PIN via a wire digitalWrite(D1, LOW);  //-->RESET
  
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(LED, OUTPUT);
  // Set outputs to LOW
  digitalWrite(LED, LOW);
 Serial.print("VERSION = ");
  Serial.println(VERSION);
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
  Serial.println("Read internal Voltage = " + ESP.getVcc()); // Exerise display the voltage on the website
  mqttclient.setServer(mqtt_server, 1883);
  timeClient.begin();
  timeClient.update();
  //long timeOffset for EST
  Serial.println(timeClient.getFormattedTime());

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

  int currentMonth = ptm->tm_mon + 1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  int currentYear = ptm->tm_year + 1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

}

void postdata(String Data) {
  WiFiClient rnClient;
 // rnClient.connect(host, httpPort)
  if (rnClient.connect("52.117.240.201", 1880))
  {
    rnClient.print("POST /update HTTP/1.1\n");
    rnClient.print("Host: 52.117.240.201:1880/myesp8266\n");
    rnClient.print("Connection: close\n");
    rnClient.print("Content-Type: application/x-www-form-urlencoded\n");
    rnClient.print("Content-Length: ");
    rnClient.print(Data.length());
    rnClient.print("\n\n");
    rnClient.print(Data);
    rnClient.println("uploaded to server....");
    
  } rnClient.stop();
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
void WiFiOn() {

  wifi_fpm_do_wakeup();
  wifi_fpm_close();

  Serial.println("Reconnecting");
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();
}


void WiFiOff() {

  Serial.println("diconnecting client and wifi");
 // client.disconnect();
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);
  wifi_set_sleep_type(MODEM_SLEEP_T);
  wifi_fpm_open();
 // wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);

}

//
// Create MQTT connection
//

void reconnect() {
  delay(1000);
  // Loop until we're reconnected
  Serial.print("MQTT connected = ");
  Serial.println(mqttclient.connected());
  delay(2000);
  while (!mqttclient.connected()) {
    mqttclient.setServer(mqtt_server, 1883);
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
      mqttmsg = "{\"hello\":\"" + DEVICEID + "\"}";
      String mytopic =  "sensor/" + DEVICEID + "/hello";
      mqttclient.publish(mytopic.c_str (), mqttmsg.c_str(), true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 15 seconds");
      // Wait 15 seconds before retrying
      delay(15000);
    }
  }
}
void lightsleep() {
  
  Serial.println(F("\nSleep, wake in 30 seconds"));
  WiFiOff();
  WiFi.mode(WIFI_OFF);  // you must turn the modem off; using disconnect won't work
  digitalWrite(LED, HIGH);  // turn the LED off so they know the CPU isn't running
  extern os_timer_t *timer_list;
  timer_list = nullptr;  // stop (but don't disable) the 4 OS timers
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  gpio_pin_wakeup_enable(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);  // GPIO wakeup (optional)
  // only LOLEVEL or HILEVEL interrupts work, no edge, that's an SDK or CPU limitation
 // wifi_fpm_set_wakeup_cb(wakeupCallback); // set wakeup callback
  // the callback is optional, but without it the modem will wake in 10 seconds then delay(10 seconds)
  // with the callback the sleep time is only 10 seconds total, no extra delay() afterward
  wifi_fpm_open();
  wifi_fpm_do_sleep(30E6);  // Sleep range = 10000 ~ 268,435,454 uS (0xFFFFFFE, 2^28-1)
  delay(30e3 + 1); // delay needs to be 1 mS longer than sleep or it only goes into Modem Sleep
  Serial.println(F("Woke up!"));  // the interrupt callback hits before this is executed
  WiFiOn();
  
}



//
// loop
//
void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  currentTime1 = millis();
  //previousTime1 = currentTime1;


  if ((currentTime1 - previousTime1) > mqttSchedule ) // send all 5sec
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
      h = 0;
      t = 0;
    }
    //WiFi.localIP() // IP address
    //macToStr(mac);  // Mac Address
    //timeClient.getEpochTime() // timestamp
    String mymac = macToStr(mac);
    String myip = WiFi.localIP().toString().c_str();
    long mydts = timeClient.getEpochTime();
    float myvol = ESP.getVcc();
     myvol = myvol / 1000;
    msgcounter++;
    //###EXCERISE add a message counter (msgcnt) to the json message like \"msgcounter\" : "+String(msgcounter)+ "
    mqttmsg = "{\"msg\" : "+String(msgcounter)+ ",\"mac\" :  \"" + mymac + "\",\"ip\" : \"" + myip + "\",\"voltage\" :  " + myvol + ",\"dts\" : " + mydts + ",\"deviceid\" : \"" + DEVICEID + "\", \"humidity\" : " + String(h) + " ,\"temperature\" : " + String(t) + " ,\"lat\" :" + String(LATITUDE) + ",\"lon\" : " + String(LONGITUDE) + "}";

    if (prevTemp == t)
    {
    Serial.println("Sending Sensor Data");
    Serial.println(mqttmsg.c_str());
    String mytopic = "sensor/" + DEVICEID + "/data";
   
       
    mqttclient.publish(mytopic.c_str (), mqttmsg.c_str(), true);
    postdata(mqttmsg); //test Poast Data
    }else{
      Serial.println("Same temperature no data to send!");
   //   Serial.println("Lets sleep for 30 sec");
     // lightsleep();
      }
    previousTime1 = millis();
    prevTemp = t;
  }

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;


    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,


     
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        t = dht.readTemperature();

        //Serial.print("Time = ");
        //Serial.println(timeClient.getFormattedTime());

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
             // LEDState = "on";
              digitalWrite(LED, LOW);
            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("LED off");
             // LEDState = "off";
              digitalWrite(LED, HIGH);
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

   // client.println("<p>"+String(digitalRead(LED))+"</p>");
              if(digitalRead(LED) == HIGH)
     LEDState = "off";
  else  
     LEDState = "on";

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server - " + DEVICEID + "</h1>");


/*
 * Walk thru
 *
            if (header.indexOf("GET /deepsleep") >= 0) {
                client.println("<h1>GOING TO SLEEP!</h1>");
                client.println("<p id=countdown>0</p>");
                client.println("<script>");
                client.println("(function countdown(remaining) {if(remaining <= 0) window.location.href='/refresh'; document.getElementById('countdown').innerHTML = 'Refresh in '+remaining;setTimeout(function(){ countdown(remaining - 1); }, 1000);})(60);");
                client.println("</script>");
                client.println("</body></html>");
                
               client.stop();
              Serial.println("Client disconnected.");
             
              Serial.println("Enable sleep for 30 sec");
              
              lightsleep(); // If the wifi connection can be restore lets stay 
        
              // Serial.begin(115200);
             //  Serial.println("Restart???");
              // ESP.restart(); //reset or we could to a high on D0 and reset
              Serial.println("D1 = low ");
              digitalWrite(D1, LOW); // hard Reboot via wire from D1 to RST
            }
     */       


            // Display current state, and ON/OFF buttons for GPIO 16
            client.println("<p>GPIO16 LED - State " + LEDState + "</p>");
            // If the output5State is off, it displays the ON button
            if (LEDState == "off") {
              client.println("<p><a href=\"/led/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("<div class=\"temperature\">" + timeClient.getFormattedTime() + "</div><br>");

            client.println("<p>prevTemp     = " + (String)prevTemp + "</p>");
            client.println("<p>msgcounter     = " + (String)msgcounter + "</p>");

            if (prevTemp == t)
              client.println("<div class=\"temperature\">NO Temperature Change</div><br>");
            else
              client.println("<div class=\"temperature\">Temperature Changed from " + (String)prevTemp + "</div><br>");

            prevTemp = t;
            client.println("<div class=\"side-by-side temperature-text\">Temperature</div><div class=\"side-by-side temperature\">" + (String)t + " C</div><br>");
            client.println("<div class=\"side-by-side humidity-text\">Humidity</div><div class=\"side-by-side humidity\">" + (String)h + " %</div>\n");
            //###>>EXERISE - display the voltage
            myvol = ESP.getVcc();
            myvol = myvol / 1000;

            client.println("<br><div class=\"side-by-side humidity-text\">Voltage</div><div class=\"side-by-side humidity\">" + String(myvol) + " V</div>\n<br>");
            /*
              client.println("<p>Temperature  = "+(String)t+"</p>");
              client.println("<p>Humidity     = "+(String)h+"</p>");
              client.println("<p>getEpochTime = "+String(timeClient.getEpochTime())+"</p>");
              client.println("<p>Time         = "+String(timeClient.getFormattedTime())+"</p>");
              client.println("<p>msgcounter   = "+String(msgcounter)+"</p>");
            */   client.println("<p id=countdown>0</p>");
            client.println("<p><a href=\"/refresh\"><button class=\"button button2\">Refresh</button></a></p>");
            client.println("<p><a href=\"/deepsleep\"><button class=\"button\">Deepsleep for 30sec</button></a></p>");

            client.println("<hr><p>mqttmsg = " + mqttmsg + "</p>");
            
            // auto refesh page every 60sec
            client.println("<script>");
            // auto refresh <<<TODO
          //  client.println("(function countdown(remaining) {if(remaining <= 0) window.location.href='/refresh'; document.getElementById('countdown').innerHTML = 'Refresh in '+remaining;setTimeout(function(){ countdown(remaining - 1); }, 1000);})(30);");
            client.println("</script>");
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
