//   Notes: 
//   PIR sensor is working on default pin D3. 
//   Real time clock auto adjust with NTP pool internet server Weird but working so/so. 
//   Zin forking switch idea: 
//   Random Seed start with a number 20 then shuffels the numbers 0 to 20.  
//   Our random salt is then suffled in our deck in parallel with a range of 1 to 7 max.  
//   then the random salt number is passed to our switch statment witch is also randomly,
//   slecting a value case 0 to 20 maximum to logically execure until break ends statment. 
//   now if in the event our salt matches numbers from 1 to 7 it appends lines of code to run.
//   Also added BME280 Temp/pressure sensor to main 1x3 board. Tactile push buttons x2 added on the
//   back of main 1x3 board. 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <time.h>
#include <NTPClient.h> 
#include <WiFiUdp.h> 
#include "database.h"
#include "MLEDScroll.h"
#include "SSD1306Wire.h"
#include "ClosedCube_SHT31D.h"
#include <RTClib.h>
#include <Arduino.h> 
#include <Bme280.h> 
#define buttonPin1 D8 
#define buttonPin2 D0 
WiFiUDP wifiUdp; 
NTPClient timeClient(wifiUdp, ntpserver4, -7 * 3600, 60000); 
Bme280TwoWire sensor; 
const int PIR = D3;
int PIRState = 0;
int pinStateCurrent   = LOW; 
int pinStatePrevious  = LOW; 
int buttonState = 0;  
SSD1306Wire display(0x3c, SDA, SCL); 
ESP8266WebServer server(80);
RTC_DS1307 rtc; 
MLEDScroll matrix;
ClosedCube_SHT31D sht3xd;

void setup() {
Serial.begin(115200);
#ifndef ESP8266
while(!Serial); 
#endif 
pinMode(PIR, INPUT);
pinMode(buttonPin1, INPUT); 
pinMode(buttonPin2, INPUT);
digitalWrite(buttonPin1, LOW); 
digitalWrite(buttonPin2, LOW); 
Wire.begin(D2, D1);  

for(int x=0; x<5; x++) { Serial.println("."); } Serial.println(); 
Serial.print("Running Real Time Clock tests..."); 
if(! rtc.begin()) { 
  Serial.println("Couldn't find RTC"); 
  Serial.flush();  
  } 
Serial.print("..");   
if(!rtc.isrunning()) { 
lostpower();   
Serial.println("RTC is running [ok]"); }
Serial.println("...Done."); 

Serial.print("Systems startup");
for(int x=0; x<3; x++) { 
Serial.print(".");  delay(100); 
if (x == 3) { Serial.println("."); } 
delay(100); 
}
Serial.println("Serial @ 115200 Bps"); 
Serial.print("Active Bme280 Sensor I2C primary test.."); 
sensor.begin(Bme280TwoWireAddress::Primary);
sensor.setSettings(Bme280Settings::indoor());
Serial.println(".Finished tests. I2C address: 0x76"); 
Wire.begin(); Serial.println("Power cycle starting.");
sht3xd.begin(0x45); Serial.println("SHT30 sensor not address 0x44.. I2C address: 0x45");  
if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR) Serial.println("[ERROR] Cannot start periodic mode");
matrix.begin(); 
matrix.flip = false; 
Serial.println("Dot Matrix 8x8 Startup cycle.."); 
for(int x=1; x<3; x++) {
matrix.icon(0x103);
delay(300); 
matrix.clear(); 
delay(300);  
matrix.icon(0x103);
delay(300); 
}
matrix.clear();  
delay(100); 
Serial.print("OLED display startup, I2C device address 0x3c."); 
display.init(); 
display.setContrast(255);
display.setTextAlignment(TEXT_ALIGN_CENTER);
display.flipScreenVertically();
display.setFont(ArialMT_Plain_10);
display.setTextAlignment(TEXT_ALIGN_CENTER);
Serial.println("...Done.\nTesting OLED display & 8x8 LED matrix."); 
for(int x=0; x<15; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep);
display.drawString(65, 50, String("Loading.."));
display.drawString(65, 53, String(""));
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep2 );
display.drawString(65, 50, String("Loading...."));
display.drawString(65, 53, String(""));
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}

display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
matrix.icon(0x103);
display.display();  
WiFi.mode(WIFI_STA); 
WiFi.begin(ssid, password); 
while(WiFi.status() != WL_CONNECTED) {  
  matrix.icon(0x104); delay(109); 
  Serial.print("."); matrix.clear(); 
  matrix.icon(0x105); delay(109); matrix.clear();  
}
Serial.println("...test finished."); 
Serial.print("NTP/RTC clock updateing.."); 
timeClient.begin(); Serial.print(".."); 
timeClient.update(); Serial.print(".."); 
timeClient.forceUpdate(); Serial.print(".."); 
timeClient.setTimeOffset(-7 * 3600); Serial.println("..Done."); 
matrix.clear();  
Serial.print("Starting up server..."); 
 server.begin();
 server.on("/", handleRoot); 
 server.on("/version", handleVersion); 
 server.on("/weather", handleweather2); 
 server.on("/networks", handlenetwork); 
 server.on("/panic", handlepanic); 
 server.on("/shutdown_panic", shutdown_panic); 
 server.on("/ping", Stamp); 
 server.on("/RTC", rtcstamp); 
 server.on("/ip", ipstamp); 
 server.onNotFound(handleNotFound);
for(int x=0; x<4; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
display.clear(); 
display.display(); 
display.drawString(65, 36, String("SERVER"));
display.drawString(65, 45, String("WAKE"));
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_alert);
display.display(); 
delay(487); 
display.clear(); 
display.display(); 
display.drawString(65, 36, String("SERVER"));
display.drawString(65, 45, String("WAKE"));
display.display(); 
delay(400);  
display.clear(); 
display.display(); 
}
Serial.print("..done.");  
matrix.icon(0x102); 
delay(609);
 matrix.flip = false;
matrix.setIntensity(1);
  matrix.message("Ready     ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(20);
    yield();
  }
 matrix.clear(); 
 display.clear(); 
 display.display(); 
 display.setFont(ArialMT_Plain_10);
 display.drawString(65, 33, String("READY"));
 display.drawString(65, 45, String(" "));
 display.display(); 
 delay(1000); 
 display.clear(); 
 display.display(); 
 delay(1000); 
 Serial.println("[READY]"); 
 Serial.println("Random Seed slect started.."); 
 randomSeed(20);
 Serial.println("[OK!] Exiting setup functions.");
 lostpower(); // Trigger update now! 3===>  levels balls deep!
 TimeStamp(); 
}

int delayTime = 100; 
long int j = 0; 
long int t = 0; 
void loop() { j++;  
  button_trap();
  auto temperature = String(sensor.getTemperature()) + " °C";
  auto pressure = String(sensor.getPressure() / 100.0) + " hPa";
  String measurements = temperature + ", " + pressure + " ";
  Serial.println("BME280 [OK!] measurements = "+measurements);
 button_trap();
 pinStatePrevious = pinStateCurrent;
 PIRState = digitalRead(PIR);
 if (PIRState == HIGH) { printResult("Periodic Mode", sht3xd.periodicFetchData()); } 
 button_trap();
 display.setContrast(255);
 display.setTextAlignment(TEXT_ALIGN_CENTER);
 display.flipScreenVertically();
 display.setFont(ArialMT_Plain_10);
 display.setTextAlignment(TEXT_ALIGN_CENTER);
 display.clear(); 
 display.display(); 
 catzshow(random(1,7)); 
 for(int x = 0; x<2; x++) {
 display.clear(); 
 display.drawXbm(34, 14, OLEDICON_width, OLEDICON_height, INFO_Icon);
 display.display();   
 button_trap();
 server.handleClient(); 
 MDNS.update();
 delay(30); 
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 button_trap();
 }
 }
 catzshow(random(1,7)); 
 display.clear(); 
 display.drawXbm(34, 14, OLEDICON_width, OLEDICON_height, INFO_Icon);
 display.display();   
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 button_trap();
 }
 display.clear(); 
 display.display(); 
 catzshow(random(1,7)); 
 for(int x = 0; x<2; x++) {
 display.clear(); 
 display.drawXbm(37, 14, OLEDICON_width, OLEDICON_height, Network_logo);
 display.display(); 
 server.handleClient(); 
 MDNS.update();
 delay(30); 
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 button_trap();
 }
 } 
if (j == 2) {
j = 0;   
for(int x=0; x<16; x++) { server.handleClient(); MDNS.update(); }
} 
  catzshow(random(1,7)); 
pinStateCurrent = digitalRead(PIR);
if (pinStatePrevious == LOW && pinStateCurrent == HIGH) { 
  catzshow(random(1,7)); 
  printResult("PIR Sensor active", sht3xd.periodicFetchData()); 
}
else 
if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {  catzshow(random(1,7)); } 
TimeStamp(); 
button_trap();    
}

void button_trap() {
   buttonState = LOW; 
   digitalWrite(buttonPin1, LOW);
   digitalWrite(buttonPin2, LOW);  
   buttonState = digitalRead(buttonPin1);
  if (buttonState == HIGH) {
    catzshow(random(1,5)); 
  } else {
  buttonState = LOW;
  digitalWrite(buttonPin1, LOW);
  digitalWrite(buttonPin2, LOW);     
  buttonState = digitalRead(buttonPin2);   
  }
  buttonState = digitalRead(buttonPin2);
  if (buttonState == HIGH) {
    catzshow(random(1,5)); 
  }
  
    buttonState = LOW; 
    digitalWrite(buttonPin1, LOW);
    digitalWrite(buttonPin2, LOW);  
    buttonState = digitalRead(buttonPin2);   
  if (buttonState == HIGH) {
    catzshow(random(1,5)); 
  } else {
      buttonState = LOW; 
      digitalWrite(buttonPin1, LOW);
      digitalWrite(buttonPin2, LOW);  
      buttonState = digitalRead(buttonPin1);
  }
      buttonState = digitalRead(buttonPin1);
     if (buttonState == HIGH) {
  catzshow(random(1,5)); 
  }
      buttonState = LOW; 
      digitalWrite(buttonPin1, LOW);
      digitalWrite(buttonPin2, LOW);  
}

void catzshow(int j) {
switch (random(0,20)) {
  case 0: 
  if (j == 3) {
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400);   
  }
  for(int x=0; x<9; x++) {
server.handleClient(); 
MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink2);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}
  break;     
  case 1: 
if (j == 1) {
    for(int x=0; x<3; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
}  
}
for(int x=0; x<5; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
}
  break;
  case 2: 
  if (j == 2) {
      for(int x=0; x<2; x++) {
server.handleClient(); 
MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
server.handleClient(); 
MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run3);
display.display(); 
delay(400); 
display.clear(); 
display.display();  
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
}
  }
for(int x=0; x<5; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake1);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
}
  break; 
  case 3:
  if (j == 3) {
    display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
  }
for(int x=0; x<6; x++) {
   server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run3);
display.display(); 
delay(400); 
display.clear(); 
display.display();
}
  break;
  case 4:
  if (j == 4) {
server.handleClient(); 
MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_board);
display.display(); 
delay(400); 
display.clear(); 
display.display();  
  }
  for(int x=0; x<6; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
}
  break; 
  case 5:
  if (j == 5) {
    display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 server.handleClient(); 
 MDNS.update();
  }
  for(int x=0; x<6; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_board);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}
  break; 
  case 6: 
  if (j == 6) {
server.handleClient(); 
MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink);
display.display(); 
delay(400);  
  }
  for(int x=0; x<6; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}
  break; 
  case 7: 
  if (j == 7) {
server.handleClient(); 
MDNS.update();
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_alert);
display.display(); 
delay(400);
  }
  for(int x=0; x<9; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
 server.handleClient(); 
 MDNS.update();
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_alert);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake2);
display.display(); 
delay(400); 
display.clear(); 
display.display();
}
  break; 
  case 8:
  if (j == 1) {
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
  }
  for(int x=0; x<4; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_lookingDown);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_lookingDown);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}
  break; 
  case 9: 
  for(int x=0; x<3; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_dig1);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_board);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_dig1);
display.display(); 
delay(400); 
display.clear(); 
display.display();
}
  break; 
  case 10: 
  for(int x=0; x<6; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink);
display.display(); 
delay(400); 
display.clear(); 
display.display();
}
  break; 
  case 11: 
  for(int x=0; x<10; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
}
  break; 
  case 12: 
  for(int x=0; x<2; x++) {
     server.handleClient(); 
 MDNS.update();
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake2);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_alert);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
display.clear(); 
display.display();
}
  break; 
  case 13:
  if (j == 2) {
     matrix.setIntensity(1);
 matrix.message("Connect via a phone or smart device at ip address "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
  }
  for(int x=0; x<5; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run3);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run2);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wink2);
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_run1);
display.display(); 
delay(400); 
display.clear(); 
display.display();
 server.handleClient(); 
 MDNS.update();
}
  break; 
  case 14: 
  if (j == 6) {
    display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
     matrix.setIntensity(1);
 matrix.message("Hello  ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
  }
    for(int x=0; x<2; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
 server.handleClient(); 
 MDNS.update();
}
  break;
  case 15:
  if (j == 7) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
     matrix.setIntensity(1);
 matrix.message("Server active ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
  }
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_board);
display.display(); 
delay(400);  
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
  break; 
  case 16: 
if (j == 4) {
   matrix.setIntensity(1);
 matrix.icon(0x101);   
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }  
}
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep);
display.display(); 
delay(400);  
 server.handleClient(); 
 MDNS.update();
  break; 
  case 17: 
  if (j == 1) {
    display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_board);
display.display(); 
delay(400); 
  matrix.setIntensity(1);
 matrix.icon(0x101);   
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }  
  }
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400);  
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
  break; 
  case 18: 
  if(j == 3) {
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
 matrix.setIntensity(1);
 matrix.icon(0x101);   
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }  
  }
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep2);
display.display(); 
delay(400);  
  break; 
  case 19: 
if(j == 3) {
  display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
   matrix.setIntensity(1);
 matrix.icon(0x101);   
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }    
}
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sleep);
display.display(); 
delay(400);  
  break; 
  case 20: 
  if(j == 6) {
for(int x = 0; x<4; x++) {
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways2);
display.display(); 
delay(400); 
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_sideways1);
display.display(); 
delay(400);
}
for(int x=0; x<9; x++) {
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_lookingDown);
display.display(); 
delay(400);
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_dig1);
display.display(); 
delay(400);
display.clear();  
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_dig2);
display.display(); 
delay(400);
matrix.setIntensity(1);
matrix.icon(0x101);   
matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }   
}
  }
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_wake);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
display.drawXbm(34, 14,cat_width, cat_height, cat_alert);
display.display(); 
delay(400); 
  break; 
  default: 
 display.clear(); 
 display.display(); 
 for(int x = 0; x<2; x++) {
 display.clear(); 
 display.drawXbm(34, 14, OLEDICON_width, OLEDICON_height, INFO_Icon);
 display.display();   
 server.handleClient(); 
 MDNS.update();
 delay(30); 
 matrix.setIntensity(1);
 matrix.message(" "+WiFi.localIP().toString()+"    ");
  while (matrix.scroll() != SCROLL_ENDED) {
    delay(99);
    yield();
 server.handleClient(); 
 MDNS.update();
 }
 }
  break; 
}

}

void handleVersion() {
String about_message = "<!DOCTYPE html>";
       about_message += "<html lang='en'>"; 
       about_message += "<head>"; 
       about_message += "<title>Project: DATADOG the update logs.</title>"; 
       about_message += "<meta charset='utf-8'>";
       about_message += "<meta name='viewport' content='width=device-width, initial-scale=2'>";
       about_message += "<style>"; 
       about_message += "body {"; 
       about_message += "background-color: blue;"; 
       about_message += "background: rgb(2,0,36);";
       about_message += "background: linear-gradient(90deg, rgba(2,0,36,1) 13%, rgba(1,1,85,1) 36%, rgba(0,212,255,1) 100%);";          
       about_message += "}";
       about_message += "h3 {"; 
       about_message += "text-shadow: 2px 2px 0 #bcbcbc, 4px 4px 0 #9c9c9c;";
       about_message += "text-outline: 2px 2px 0px #bcbcbc, 4px 4px 0 #9c9c9c;";
       about_message += "text-color: blue;";
       about_message += "}"; 
       about_message += ".body_sec {"; 
       about_message += "position: center;"; 
       about_message += "border-radius:40px;"; 
       about_message += "background:white;"; 
       about_message += "background-position: left top;"; 
       about_message += "bacground-repeat: repeat"; 
       about_message += "width: 61%;"; 
       about_message += "padding-right: 9px;"; 
       about_message += "padding-left: 9px;"; 
       about_message += "}";
       about_message += "</style>";
       about_message += "<body>";
       about_message += "<font color='red' size='15px'>"; 
       about_message += "<center></br>Programming by,</font>";
       about_message += "<font color='red' size='20px'></br><h3>Johnny B Stroud</h3></br></center></font>";     
       about_message += "<main class='body_sec'>";
       about_message += "</br><ol>";
       about_message += "<li>Firmware 1.0Bata</li>";
       about_message += "<p>A simple device framework and/or platform for a Internet Of Things device.</p>"; 
       about_message += "<li>"; 
       about_message += "Last update log: Dec 22, 2022 * OLED artwork updated.</li></br>";
       about_message += "<p>Added space invaders and oled lolcat to this sketch.</p></br>";
       about_message += "<li>Last update log: Dec 23, 2022 * 8x8 RED led matrix icons added.</li></br>";
       about_message += "<p>Power icon flashes for power cycle and setup of server. jittering bug fixed with MLEDScrller cpp library.</p></br>";
       about_message += "<br/><li>Last update log: Jan 20, 2023 <br/> [Hidden Panic mode update]</li> "; 
       about_message += "<p>Panic mode offers a remote shutdown button for the ESP8266 WiFi.  </p>";
       about_message += "<li>Last update log: Mar, 10 2022 * Hardware upgrade BME280 chip</li>";
       about_message += "<p>I also added 2 tacktile push buttons to the main board. I ";
       about_message += "finished an update to weather. Working with BME280 & SHT30 chips.</p>";
       about_message += "</ol>";  
       about_message += "</main>"; 
       about_message += "</body>"; 
       about_message += "</html>"; 
       about_message += "";
server.send(200, "text/html", about_message);  
Serial.println("[200] Server sending ok."); 
for(int x=0; x<10; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14, invader1_width, invader1_height, invader1_bits);
display.drawString(65, 47, String("Dec 22,2022"));
display.drawString(65, 52, String("Johnny B"));
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14, invader2_width, invader2_height, invader2_bits);
display.drawString(65, 47, String("Dec 22,2022"));
display.drawString(65, 53, String("Johnny B"));
display.display(); 
delay(400); 
}

for(int x=0; x<10; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14, invader1_width, invader1_height, invader1_bits);
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14, invader2_width, invader2_height, invader2_bits);
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}
for(int x=0; x<10; x++) {
display.clear(); 
display.display(); 
display.drawXbm(34, 14, invader1_width, invader1_height, invader1_bits);
display.drawString(65, 47, String("VERSION"));
display.drawString(65, 52, String("1.0Bata"));
display.display(); 
delay(400);  
display.clear();  
display.display(); 
display.drawXbm(34, 14, invader2_width, invader2_height, invader2_bits);
display.drawString(65, 47, String("VERSION"));
display.drawString(65, 53, String("1.0Bata"));
display.display(); 
delay(400); 
display.clear(); 
display.display(); 
}

}

void handleRoot() {
  String indexhtml =  "";
         indexhtml += "";   
indexhtml += "<!DOCTYPE html>";
indexhtml += "<html lang='en'>";
indexhtml += "<head>";
indexhtml += "<title>DataDog server</title>";
indexhtml += "<meta charset='utf-8'>";
indexhtml += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
indexhtml += "<style>";
indexhtml += "* {";
indexhtml += "  box-sizing: border-box;";
indexhtml += "}";
indexhtml += "body {";
indexhtml += "  font-family: Arial, Helvetica, sans-serif;";
indexhtml += "}";
indexhtml += "header {";
indexhtml += "  background-color: #666;";
indexhtml += "  padding: 30px;";
indexhtml += "  text-align: center;";
indexhtml += "  font-size: 35px;";
indexhtml += "  color: white;";
indexhtml += "}";
indexhtml += "nav {";
indexhtml += "  float: left;";
indexhtml += "  width: 30%;";
indexhtml += "  height: 300px; ";
indexhtml += "  background: #ccc;";
indexhtml += "  padding: 20px;";
indexhtml += "}";
indexhtml += "nav ul {";
indexhtml += "  list-style-type: none;";
indexhtml += "  padding: 0;";
indexhtml += "}";
indexhtml += "article {";
indexhtml += "  float: left;";
indexhtml += "  padding: 20px;";
indexhtml += "  width: 70%;";
indexhtml += "  background-color: #f1f1f1;";
indexhtml += "  height: 300px;";
indexhtml += "}";
indexhtml += "section::after {";
indexhtml += "  content: '';";
indexhtml += "  display: table;";
indexhtml += "  clear: both;";
indexhtml += "}";
indexhtml += "footer {";
indexhtml += "  background-color: #777;";
indexhtml += "  padding: 10px;";
indexhtml += "  text-align: center;";
indexhtml += "  color: white;";
indexhtml += "}";
indexhtml += "@media (max-width: 600px) {";
indexhtml += "  nav, article {";
indexhtml += "    width: 100%;";
indexhtml += "    height: auto;";
indexhtml += "  }";
indexhtml += "}";
indexhtml += "</style></head><body>";
indexhtml += "<h2>DATADOG Server version 1.0 Bata</h2>";
indexhtml += "<p>D1 Mini pro ESP8266 IOT device with 16MB of flash memory.";
indexhtml += "Real Time Clock and SD datalogger is active on this device.";
indexhtml += "";
indexhtml += "</p>"; 
indexhtml += "<header>";
indexhtml += "  <h2><center>Digital Archives Time And Data Onboard Groups</center></h2>";
indexhtml += "</header>";
indexhtml += "<section>";
indexhtml += "  <nav>";
indexhtml += "    <ul>";
indexhtml += "      <li><a href='/'>About Sever</a></li>";
indexhtml += "      <li><a href='networks'>Network map</a></li>";
indexhtml += "      <li><a href='version'>Version log</a></li>";
indexhtml += "      <li><a href='weather'>Weather</a></li>";
indexhtml += "    </ul>";
indexhtml += "  </nav>";  
indexhtml += "  <article>";
indexhtml += "    <h1>About this server</h1>";
indexhtml += "    <p>The ESP8266 is a low-cost Wi-Fi microchip, with built-in TCP/IP networking software, and microcontroller capability, produced by Espressif Systems in Shanghai, China.";
indexhtml += "The chip was popularized in the English-speaking maker community in August 2014 via the ESP-01 module, made by a third-party manufacturer Ai-Thinker.";
indexhtml += "This small module allows microcontrollers to connect to a Wi-Fi network and make simple TCP/IP connections using Hayes-style commands. However, at first, there was almost no English-language documentation on the chip and the commands it accepted.";
indexhtml += "</p>";
indexhtml += "    <p></p>";
indexhtml += "  </article>";
indexhtml += "</section>";
indexhtml += "<footer><p>Firware version 1.0 Bata | By Johnny B Stroud</p></footer>";
indexhtml += "</body></html>";

    Serial.println("[200] text/html root send"); 
    server.send(200, "text/html", indexhtml); 
    catzshow(6); 
}

void handleNotFound() {
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
  Serial.println(message); 
  Serial.println("[404] Server responding to request."); 
  catzshow(1); 
}

void handlenetwork() {

  String index2html =  "";
         index2html += "";   
index2html += "<!DOCTYPE html>";
index2html += "<html lang='en'>";
index2html += "<head>";
index2html += "<title>Network scanning [DataDog server]</title>";
index2html += "<meta charset='utf-8'>";
index2html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
index2html += "<style>";
index2html += "* {";
index2html += "  box-sizing: border-box;";
index2html += "}";
index2html += "body {";
index2html += "  font-family: Arial, Helvetica, sans-serif;";
index2html += "}";
index2html += "header {";
index2html += "  background-color: #666;";
index2html += "  padding: 30px;";
index2html += "  text-align: center;";
index2html += "  font-size: 35px;";
index2html += "  color: white;";
index2html += "}";
index2html += "nav {";
index2html += "  float: left;";
index2html += "  width: 30%;";
index2html += "  height: 300px; ";
index2html += "  background: #ccc;";
index2html += "  padding: 20px;";
index2html += "}";
index2html += "nav ul {";
index2html += "  list-style-type: none;";
index2html += "  padding: 0;";
index2html += "}";
index2html += "article {";
index2html += "  float: left;";
index2html += "  padding: 20px;";
index2html += "  width: 70%;";
index2html += "  background-color: #f1f1f1;";
index2html += "  height: 300px;";
index2html += "}";
index2html += "section::after {";
index2html += "  content: '';";
index2html += "  display: table;";
index2html += "  clear: both;";
index2html += "}";
index2html += "footer {";
index2html += "  background-color: #777;";
index2html += "  padding: 10px;";
index2html += "  text-align: center;";
index2html += "  color: white;";
index2html += "}";
index2html += "@media (max-width: 600px) {";
index2html += "  nav, article {";
index2html += "    width: 100%;";
index2html += "    height: auto;";
index2html += "  }";
index2html += "}";
index2html += "</style></head><body>";
index2html += "<h2><center>";
index2html += "Our netowrk ip address is ";
index2html += WiFi.localIP().toString();
index2html += "</center></h2>"; 
index2html += "<header>";
index2html += "  <h2><center>NETWORK MAP</center></h2>";
index2html += "</header>";
index2html += "<section>";
index2html += "  <nav>";
index2html += "    <ul>";
index2html += "      <li><a href='/'>About Sever</a></li>";
index2html += "      <li><a href='networks'>Network map</a></li>";
index2html += "      <li><a href='version'>Version log</a></li>";
index2html += "      <li><a href='weather'>Weather</a></li>";
index2html += "    </ul>";
index2html += "  </nav>";  
index2html += "  <article>";
index2html += "    <h1>ESSID's of Networks</h1>";
index2html += "    <p>";
 int Tnetwork=0, i=0, len=0;
  Tnetwork = WiFi.scanNetworks();       
 for (int i = 0; i < Tnetwork; ++i)
    {
   index2html +=i + 1;
   index2html += ": ";
   index2html += WiFi.SSID(i);
   index2html += " (";
   index2html += WiFi.RSSI(i);
   index2html += ")";
   index2html += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
   index2html += "<br/>";
    }
index2html += "</p>";
index2html += "    <p></p>";
index2html += "  </article>";
index2html += "</section>";
index2html += "<footer><p>Firware version 1.0 Bata | By Johnny B Stroud</p></footer>";
index2html += "</body></html>";
    server.send(200, "text/html", index2html);
    Serial.println("[200] text/html server sending"); 
    catzshow(3); 
}

void printResult(String text, SHT31D result) {
  t++; 
  if (result.error == SHT3XD_NO_ERROR) {
    Serial.println("\n"); 
    Serial.println(text);
    Serial.print("Temperature:");
    Serial.print(result.t);
    Serial.println("C");
    Serial.print("Humidity:"); 
    Serial.print(result.rh);
    Serial.println("%");
    Serial.print("Test #"); 
    Serial.print(t, DEC); 
    Serial.println(" "); 
  } else {
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(result.error);
  }
  catzshow(5); 
  byte error, address; 
  int nDevices; 
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
     Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println(" ");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    } 
  }
    if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    }
  else
  Serial.println(" ");
}

void handleweather2() {
  Serial.println("SHT30 sensor wake up! [Triggered]"); 
  handleweather(sht3xd.periodicFetchData()); 
  catzshow(random(1,7)); 
}

void handleweather(SHT31D result) {  
  float resultf = (result.t * 9.0/5.0)+32.0; 
  auto temperature = String(sensor.getTemperature()) + " °C";
  auto pressure = String(sensor.getPressure() / 100.0) + " hPa";

  String index3html =  "";
         index3html += "";   
index3html += "<!DOCTYPE html>";
index3html += "<html lang='en'>";
index3html += "<head>";
index3html += "<title>DataDog server - Weather sensor -</title>";
index3html += "<meta charset='utf-8'>";
index3html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
index3html += "<style>";
index3html += "* {";
index3html += "  box-sizing: border-box;";
index3html += "}";
index3html += "body {";
index3html += "  font-family: Arial, Helvetica, sans-serif;";
index3html += "}";
index3html += "tav{"; 
index3html += "text-shadow: 2px 2px 0 #bcbcbc, 4px 4px 0 #9c9c9c;";
index3html += "text-outline: 2px 2px 0px #bcbcbc, 4px 4px 0 #9c9c9c;";
index3html += "font-size: 44px;"; 
index3html += "font-color: black;";
index3html += "text-align: center;";  
index3html += "}";
index3html += "header {";
index3html += "  background-color: #666;";
index3html += "  padding: 30px;";
index3html += "  text-align: center;";
index3html += "  font-size: 35px;";
index3html += "  color: white;";
index3html += "}";
index3html += "nav {";
index3html += "  float: left;";
index3html += "  width: 30%;";
index3html += "  height: 300px; ";
index3html += "  background: #ccc;";
index3html += "  padding: 20px;";
index3html += "}";
index3html += "nav ul {";
index3html += "  list-style-type: none;";
index3html += "  padding: 0;";
index3html += "}";
index3html += "article {";
index3html += "  float: left;";
index3html += "  padding: 20px;";
index3html += "  width: 70%;";
index3html += "  background-color: #f1f1f1;";
index3html += "  height: 300px;";
index3html += "}";
index3html += "section::after {";
index3html += "  content: '';";
index3html += "  display: table;";
index3html += "  clear: both;";
index3html += "}";
index3html += "footer {";
index3html += "  background-color: #777;";
index3html += "  padding: 10px;";
index3html += "  text-align: center;";
index3html += "  color: white;";
index3html += "}";
index3html += "@media (max-width: 600px) {";
index3html += "  nav, article {";
index3html += "    width: 100%;";
index3html += "    height: auto;";
index3html += "  }";
index3html += "}";
index3html += "</style></head><body>";
index3html += "<h2>Weather Station</h2>";
index3html += "<p>D1 Mini pro ESP8266 has enabled its SHT30 and<br/>";
index3html += "BME280 sensors.";
index3html += "";
index3html += "</p>"; 
index3html += "<header>";
index3html += "  <h2><center>Humidity and Temperature</center></h2>";
index3html += "</header>";
index3html += "<section>";
index3html += "  <nav>";
index3html += "    <ul>";
index3html += "      <li><a href='/'>About Sever</a></li>";
index3html += "      <li><a href='networks'>Network map</a></li>";
index3html += "      <li><a href='version'>Version log</a></li>";
index3html += "      <li><a href='weather'>Weather</a></li>";
index3html += "    </ul>";
index3html += "  </nav>";  
index3html += "  <article>";
index3html += "    <h1>Humidity</h1>";
index3html += "    <p>";
index3html += "<h3><tav>";
index3html += result.rh;
index3html += "%"; 
index3html += "</tav></h3>";
index3html += "</p>";
index3html += "<p></p>";
index3html += "    <h1>Temperature</h1>";
index3html += "    <p>";
index3html += "<h3><tav>";
index3html += result.t;
index3html += " &#8451; / ";
index3html += temperature;
index3html += "<br/>";
index3html += resultf;
index3html += " &#8457; / ";
index3html += pressure; 
index3html += "<br/>"; 
index3html += "</tav></h3>";
index3html += "</p>";
index3html += "<p></p>";
index3html += "  </article>";
index3html += "</section>";
index3html += "<footer><p>Firware version 1.0 Bata | By Johnny B Stroud</p></footer>";
index3html += "</body></html>";

    server.send(200, "text/html", index3html); 
    Serial.println("[200] text/html server sending"); 
}

void handlepanic() {
  String index4html =  "";
         index4html += "";   
index4html += "<!DOCTYPE html>";
index4html += "<html lang='en'>";
index4html += "<head>";
index4html += "<title>DataDog server</title>";
index4html += "<meta charset='utf-8'>";
index4html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
index4html += "<style>";
index4html += "* {";
index4html += "  box-sizing: border-box;";
index4html += "}";
index4html += "body {";
index4html += "  font-family: Arial, Helvetica, sans-serif;";
index4html += "}";
index4html += "header {";
index4html += "  background-color: #666;";
index4html += "  padding: 30px;";
index4html += "  text-align: center;";
index4html += "  font-size: 35px;";
index4html += "  color: white;";
index4html += "}";
index4html += "nav {";
index4html += "  float: left;";
index4html += "  width: 30%;";
index4html += "  height: 300px; ";
index4html += "  background: #ccc;";
index4html += "  padding: 20px;";
index4html += "}";
index4html += "nav ul {";
index4html += "  list-style-type: none;";
index4html += "  padding: 0;";
index4html += "}";
index4html += "article {";
index4html += "  float: left;";
index4html += "  padding: 20px;";
index4html += "  width: 70%;";
index4html += "  background-color: #f1f1f1;";
index4html += "  height: 300px;";
index4html += "}";
index4html += "section::after {";
index4html += "  content: '';";
index4html += "  display: table;";
index4html += "  clear: both;";
index4html += "}";
index4html += "footer {";
index4html += "  background-color: #777;";
index4html += "  padding: 10px;";
index4html += "  text-align: center;";
index4html += "  color: white;";
index4html += "}";
index4html += "@media (max-width: 600px) {";
index4html += "  nav, article {";
index4html += "    width: 100%;";
index4html += "    height: auto;";
index4html += "  }";
index4html += "}";
index4html += "</style></head><body>";
index4html += "<h2>DATADOG Server - Panic buttons -</h2>";
index4html += "<p>  ";
index4html += "";
index4html += "";
index4html += "</p>"; 
index4html += "<header>";
index4html += "  <h2><center>Server tools active</center></h2>";
index4html += "</header>";
index4html += "<section>";
index4html += "  <nav>";
index4html += "    <ul>";
index4html += "      <li><a href='/'>About Sever</a></li>";
index4html += "      <li><a href='networks'>Network map</a></li>";
index4html += "      <li><a href='version'>Version log</a></li>";
index4html += "      <li><a href='weather'>Weather</a></li>";
index4html += "      <li><a href='panic'>Painc Mode</a></li>"; 
index4html += "    </ul>";
index4html += "  </nav>";  
index4html += "  <article>";
index4html += "    <h1>Panic Options</h1>";
index4html += "<form action='shutdown_panic' method='get'>";
index4html += "<input type='submit' value='SHUTDOWN SERVER'>"; 
index4html += "<label><p>This button will shutdown the server. You will have to do a hard reset of the device after shutdown. </p></lable>"; 
index4html += "</form>";
index4html += "    <p></p>";
index4html += "  </article>";
index4html += "</section>";
index4html += "<footer><p>Firware version 1.0 Bata | By Johnny B Stroud</p></footer>";
index4html += "</body></html>";

    Serial.println("[200] text/html index4 send"); 
    server.send(200, "text/html", index4html); 
    catzshow(random(1,7)); 
}

void shutdown_panic() {
Serial.println("Server shutdown ..");
for(int y=0; y<660; y++) {
 display.clear(); 
 display.drawXbm(34, 14, OLEDICON_width, OLEDICON_height, WiFi_Logo_bits);
 display.display(); 
 matrix.setIntensity(1);
 matrix.icon(0x103); 
 matrix.message("SHUTDOWN    ");
 while (matrix.scroll() != SCROLL_ENDED) {
    delay(19);
    yield();
WiFi.mode(WIFI_OFF); 
WiFi.forceSleepBegin();
}
for(int y=0; y<6; y++) {
display.clear(); 
display.drawXbm(34, 14, OLEDICON_width, OLEDICON_height, WiFi_Logo_bits);
display.display(); 
matrix.icon(0x103); 
delay(400); 
matrix.icon(0x101);
delay(400); 
matrix.icon(0x100); 
delay(100);    
}
}
}


String lostpower() {
    time_t rawtime = timeClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

    timeClient.update();                                      
  String timeNTP = timeClient.getFormattedTime();
 Serial.print(daysOfTheWeek[timeClient.getDay()]);
Serial.print(", ");
Serial.print(timeClient.getHours());
Serial.print(":");
Serial.print(timeClient.getMinutes());
Serial.print(":");
Serial.println(timeClient.getSeconds());
  int se = timeNTP.substring(6).toInt();                 
  int mi = timeNTP.substring(3, 5).toInt();
  int ho = timeNTP.substring(0, 2).toInt();
  int ye = yearStr.toInt(); 
  int mo = monthStr.toInt(); 
  int da = dayStr.toInt(); 

  rtc.adjust(DateTime( ye , mo , da , ho , mi , se )); 
  
  Serial.println("Finished RTC adjusted! " + String(ye) + " " + String(mo) + " " +  String(da) + " " +  String(ho) + " " +  String(mi) + " " +  String(se));

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr;
}

void TimeStamp() {
    timeClient.update();
    DateTime oldTime = rtc.now();
    Serial.println("Old " + String(oldTime.unixtime()));
    Serial.println("New " + String(timeClient.getEpochTime()));
    rtc.adjust( DateTime(timeClient.getEpochTime()) );
    DateTime now = rtc.now(); 
    Serial.println("Doing Real time tests..."); 
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(", ");
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.print(timeClient.getMinutes());
    Serial.print(":");
    Serial.println(timeClient.getSeconds()+" ");
 unsigned long epochTime = timeClient.getEpochTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);  
  String formattedTime = timeClient.getFormattedTime();
    Serial.print("Formatted Time: ");
    Serial.println(formattedTime);  
  int currentHour = timeClient.getHours();
    Serial.print("Hour: ");
    Serial.print(currentHour);  
  int currentMinute = timeClient.getMinutes();
    Serial.print("Minutes: ");
    Serial.print(currentMinute);   
  int currentSecond = timeClient.getSeconds();
    Serial.print("Seconds: ");
    Serial.println(currentSecond);  
  String weekDay = weekDays[timeClient.getDay()];
    Serial.print("Week Day: ");
    Serial.println(weekDay);    
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
    Serial.print("Month day: ");
    Serial.println(monthDay);
  int currentMonth = ptm->tm_mon+1;
    Serial.print("Month: ");
    Serial.println(currentMonth);
  String currentMonthName = months[currentMonth-1];
    Serial.print("Month name: ");
    Serial.println(currentMonthName);
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    Serial.println("calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future:"); 
    DateTime future (now + TimeSpan(7,12,30,6));
    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
    Serial.println("Done..");  
}

void Stamp() {
 Serial.println(getTimeStampString()); 
 Serial.println("[200] Server ping sent out."); 
 server.send(200, "text", getTimeStampString()+"< Server ping echoed back >");  
}

void rtcstamp() {
  Serial.println(getTimeStampString()+" < RTC update triggered >"); 
  Serial.println("[200] Server RTC now updateing."); 
  TimeStamp(); 
  server.send(200, "text", getTimeStampString()+" < Finished RTC time sync >\n< Sent output to serial >\n"+"IP ADDRESS:"+WiFi.localIP().toString()+""); 
}

void ipstamp() {
    Serial.println(getTimeStampString()+" < ip address of server triggered >"); 
    Serial.println("[200] Server ip address sent."); 
    server.send(200, "text", "TimeStamp:"+getTimeStampString()+"/ IP ADDRESS:"+WiFi.localIP().toString()+"");
}

 String getTimeStampString() {
   time_t rawtime = timeClient.getEpochTime();
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr;
}
