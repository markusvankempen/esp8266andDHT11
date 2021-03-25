/*
 * Displaying an Image as a HttpClient/Server  and Posting an Image to a external http Server - mvk@ca.ibm.com
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "1Aoffice"
#define STAPSK  "2Fast4You!"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
  String img2="image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACIAAABFCAYAAAAvtjQgAAACqUlEQVRoge2YQW7cMAxFv4suAgQoIHSbla/gK/QsOdScJVfQFWaV7UBANu3OXdhMOTIpkbQnncV8wBjYlujnL4qSZ0BUr5jF6ycMkXD+ThpALSeQD4QgLo02P2Mw39wQHjn62IglJ56Edn9w7QjJ4EzfkYgTgRhtUi0nJDdIz417DWd0R6JOXKAncyOmDCJ1eALwez3KekSkwHwPhltEMAlLonKRK1LyCto6ckRy9iQ8w15HgOXNbyQbCLc3YQtU0J5JYRAp85+x5EGdCxymp8aM8g0Nl+TMDsVAuCsHAcWnL4fZmR8yyAnDprRbagGvJXV7QxyfIxSIAkedENYcfdHjRad+Iw2kdkRyQln49GSVOvDAms01RC/mqv7GSNoKcIgLdGd4287mqD99KUD9cIuMEIBn82zZOAchfCAeGCeEH4TD9HTT7xqmMWF++XF97f0DOJdYzPiid7AeILUeILUeILUeILXuBiSyee6tNXTfVeo9IDMATNMEACjnLDai+zlnF5AF5ArAKi9Q66YIkPPixJgAZfVFq5/2TOliE4APjQSSxkls3wPiJyaACcv521l25Ne49oMPaPACkN7Oyy/BvH8svwTyGccINACYU0oopTQbvgq5SjCkGgIATit/60Vzzv9ASF4ggqkhLAD82gYkChQBMIF4gSIALhArUARgAwIAFpgWUASAt9/UkShQFACsjnyC8Gl8lEMaAE1bYtgsegRgBUopYRyFAtIAkKSuvl6gKEAXZC+QN2fq1bc7lSUgPjRWAJ4fQGCrqDnkdaBW+A9fDpRzDgOQxI2RNzEts4arHhbgjj4n7hpkoES8haRh0UD+i7RZM5RSXItgT73Pidb0HQBgL1APwAKyC8gK4AFxAXkBjtCMpfjN0zTNdP5VD1eB9gb5C/ZBYF4WLxoMAAAAAElFTkSuQmCC";
  
  String img="image/png;base64,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";
static const uint8_t simily[] PROGMEM =  { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04, 0x05, 0x0A, 0x07, 0x07, 0x06, 0x08, 0x0C, 0x0A, 0x0C, 0x0C, 0x0B, 0x0A, 0x0B, 0x0B, 0x0D, 0x0E, 0x12, 0x10, 0x0D, 0x0E, 0x11, 0x0E, 0x0B, 0x0B, 0x10, 0x16, 0x10, 0x11, 0x13, 0x14, 0x15, 0x15, 0x15, 0x0C, 0x0F, 0x17, 0x18, 0x16, 0x14, 0x18, 0x12, 0x14, 0x15, 0x14, 0xFF, 0xDB, 0x00, 0x43, 0x01, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x09, 0x05, 0x05, 0x09, 0x14, 0x0D, 0x0B, 0x0D, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x00, 0x10, 0x00, 0x10, 0x03, 0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xC4, 0x00, 0x16, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x01, 0x02, 0xFF, 0xC4, 0x00, 0x22, 0x10, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x02, 0x04, 0x05, 0x11, 0x06, 0x12, 0x07, 0x13, 0x08, 0x23, 0x21, 0x22, 0x42, 0xFF, 0xC4, 0x00, 0x19, 0x01, 0x00, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x01, 0x04, 0x05, 0x07, 0xFF, 0xC4, 0x00, 0x23, 0x11, 0x01, 0x00, 0x00, 0x06, 0x02, 0x01, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x11, 0x21, 0x31, 0x41, 0x00, 0x05, 0x03, 0x12, 0x13, 0x14, 0x51, 0x71, 0x22, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00, 0x63, 0xF2, 0x47, 0xCD, 0xEC, 0x3D, 0x0F, 0x21, 0x0C, 0x26, 0x7F, 0x88, 0x65, 0x73, 0x0E, 0xB7, 0x69, 0x48, 0xA3, 0x8B, 0x85, 0x95, 0xF5, 0x08, 0x6E, 0x8A, 0x59, 0xE8, 0x32, 0x10, 0x64, 0x99, 0x19, 0x46, 0x5D, 0xA5, 0x22, 0x3F, 0x6E, 0xA3, 0xA8, 0x07, 0x7C, 0xFA, 0x61, 0xF2, 0x76, 0x50, 0x7C, 0x96, 0x21, 0x81, 0xB8, 0x35, 0xF4, 0x90, 0xE9, 0xA6, 0x15, 0x2E, 0xAD, 0x69, 0x82, 0xD9, 0x60, 0x86, 0x92, 0xAF, 0xB5, 0x08, 0x89, 0x97, 0x6B, 0xBB, 0xE8, 0x35, 0x4F, 0xD7, 0x98, 0xE1, 0xDF, 0x37, 0xF1, 0x35, 0xBC, 0x9F, 0x47, 0x8E, 0x71, 0xFE, 0x1F, 0x94, 0xAC, 0xC2, 0xD7, 0x46, 0xFD, 0x0A, 0xD6, 0x16, 0x56, 0x84, 0xAB, 0x65, 0xC5, 0xA8, 0xED, 0xD1, 0x4C, 0x84, 0x61, 0x39, 0x6E, 0x12, 0xFE, 0x7A, 0x9E, 0xC0, 0x8D, 0x04, 0xB3, 0xE5, 0xEB, 0x20, 0x26, 0x7D, 0x70, 0x90, 0x17, 0x42, 0xB4, 0x61, 0xDD, 0xB0, 0x21, 0x71, 0x02, 0xB8, 0x6D, 0x89, 0x89, 0x26, 0x9F, 0x6A, 0x21, 0x62, 0x70, 0xDA, 0xB5, 0xD5, 0xF6, 0x7D, 0xD7, 0xF4, 0xE3, 0x57, 0x2E, 0xE3, 0xAE, 0xE1, 0x33, 0xAE, 0xB9, 0x2A, 0x95, 0x69, 0xE3, 0xFE, 0xBA, 0x59, 0x0B, 0xEB, 0xEA, 0x87, 0x55, 0x1B, 0x10, 0x5F, 0xBF, 0x5A, 0x54, 0xE2, 0x0C, 0x41, 0x12, 0xD6, 0xFA, 0x6C, 0x6C, 0x13, 0xA5, 0x0E, 0xEB, 0xAA, 0x98, 0x90, 0xF2, 0x24, 0x2D, 0x20, 0xD3, 0x81, 0x35, 0x76, 0xC4, 0x41, 0x64, 0x52, 0xB4, 0xA9, 0xCD, 0x29, 0x6F, 0x34, 0x13, 0x5F, 0xD6, 0x57, 0x26, 0xC7, 0x76, 0xCA, 0x2D, 0xED, 0xF7, 0x47, 0x97, 0x8C, 0xF1, 0xA1, 0xCE, 0x9A, 0xC8, 0xD6, 0xAD, 0x51, 0xAD, 0xBC, 0xBF, 0x4D, 0xFC, 0x85, 0x3F, 0xB1, 0x0B, 0x41, 0x20, 0x32, 0x21, 0xE2, 0x20, 0x32, 0x72, 0x8E, 0xE2, 0x22, 0x37, 0xAE, 0xDB, 0x3A, 0xD7, 0xE6, 0xB7, 0x49, 0xD3, 0x4C, 0xCF, 0xF9, 0xAB, 0x1B, 0x58, 0x3E, 0xF4, 0x0E, 0x6E, 0x59, 0x89, 0x2A, 0x00, 0xB4, 0xAD, 0x5E, 0x14, 0xCC, 0xC4, 0x12, 0xA5, 0x70, 0x98, 0x36, 0xBA, 0xB6, 0x40, 0x6F, 0x7F, 0xC3, 0x9F, 0xFF, 0xD9 };

String imgtag="<img src=data:"+img+">";
String imgtag2="<img src=data:"+img2+">";
ESP8266WebServer server(80);

const int led = 13;

void postdata(String Data) {
 
  WiFiClient rnClient;
  //WiFiClientSecure wifiSecure;
    
  // rnClient.connect(host, httpPort)
  Serial.println("Posting to thinklab15.mybluemix.net/image");

  if (rnClient.connect("thinklab15.mybluemix.net", 80))
  {
    rnClient.print("POST /image HTTP/1.1\n");
    rnClient.print("Host: thinklab15.mybluemix.net\n");
    rnClient.print("Connection: close\n");
    rnClient.print("Content-Type: text/plain\n");
    rnClient.print("Content-Length: ");
    rnClient.println(Data.length());
    rnClient.println();
    rnClient.print(Data);
    //rnClient.println();
  //  delay(2000);
   // rnClient.print("body:");
   // rnClient.print(Data);
    //rnClient.println("");
     Serial.println("post uploaded to server....");

     

  }else{
      Serial.println("post error....");
    } 

   delay(500); // Can be changed
  if (rnClient.connected()) { 
    rnClient.stop();  // DISCONNECT FROM THE SERVER
  }
} 


void postdatabinary(uint8_t Data[]) {
 
  WiFiClient rnClient;
  //WiFiClientSecure wifiSecure;
    
  // rnClient.connect(host, httpPort)
  Serial.println("Posting to thinklab15.mybluemix.net/imagebytes");
 
  if (rnClient.connect("thinklab15.mybluemix.net", 80))
  {
    rnClient.print("POST /imagebytes HTTP/1.1\n");
    rnClient.print("Host: thinklab15.mybluemix.net\n");
    rnClient.print("Connection: close\n");
                 //      "Content-Disposition: form-data; name=\"image\"; filename=\"image.png\"" + "\r\n" +

    rnClient.print("Content-Type: image/png\n");
    rnClient.print("Content-Length: ");
    rnClient.println(sizeof(Data)/sizeof(Data[0]));
    rnClient.println();
   // rnClient.write(Data);
    //rnClient.println();
  //  delay(2000);
   // rnClient.print("body:");
   // rnClient.print(Data);
    //rnClient.println("");
     Serial.println("post uploaded to server....");

     

  }else{
      Serial.println("post error....");
    } 

   delay(500); // Can be changed
  if (rnClient.connected()) { 
    rnClient.stop();  // DISCONNECT FROM THE SERVER
  }
} 
  
void handleRoot() {
  digitalWrite(led, 1);
  
  server.send(200, "text/html", "<html>hello from esp8266!<br><br>"+imgtag+"<br><p><a href=\"/postimage\"><button >Post Image 1</button></a></p>"+imgtag2+"<a href=\"/postimage2\"><button >Post Image 2</button></a></p></html>");
  
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/postimage", []() {
    postdata(img);
    server.send(200, "text/html", "<htmp>Posted<a href=\"/\"><button >Back</button></a></p></html>");
  });

  server.on("/postimage2", []() {
    postdata(img2);
    server.send(200, "text/html", "<htmp>Posted Image2<a href=\"/\"><button >Back</button></a></p></html>");
  });
  server.on("/gif", []() {
    static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
    };
    char gif_colored[sizeof(gif)];
    memcpy_P(gif_colored, gif, sizeof(gif));
    // Set the background to a random set of colors
    gif_colored[16] = millis() % 256;
    gif_colored[17] = millis() % 256;
    gif_colored[18] = millis() % 256;
    server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
    
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
