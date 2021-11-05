// 
//  Project Name: White Cat [FIN]
//  Sketch Filename: WhiteCatESP32.ino 
//  Include Defs. Filename: Array_db.h 
//  Database images Filename: catimages.h
//  Version codename: Cat & Mouse
// 
///////////////////

#include <WiFi.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <WiFiAP.h> 
#include <Ethernet.h>
#include "catimages.h"
#include "Array_db.h"
#include <M5StickC.h> 

// Set your Static IP address 
IPAddress local_IP(192, 168, 1, 184); 
IPAddress gateway(192, 168, 1, 1);
IPAddress LocalServer(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8); 
IPAddress secondaryDNS(8, 8, 4, 4); 

//////////////////HACKING ZONE////////////////
// Setip esp pico WiFi payload 
// used Wi-Fi channels (available: 1-14)
const uint8_t channels[] = {random(1, 14)}; // Or we can target {2, 7, 13}
const bool wpa2 = false; // true / false options 
extern "C" {
  #include "esp_wifi.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}
// run-time variables
char emptySSID[32];
char str[32]; 
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00,                         // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00,                                     // Fragment & sequence number (will be done by the SDK)
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0xe8, 0x03,                                     // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00,                                     // capabilities Tnformation

  // Tagged parameters

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20,                                     // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,                                       // SSID

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08,                                     // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82,                                                // 1(B)
  /* 73 */ 0x84,                                                // 2(B)
  /* 74 */ 0x8b,                                                // 5.5(B)
  /* 75 */ 0x96,                                                // 11(B)
  /* 76 */ 0x24,                                                // 18
  /* 77 */ 0x30,                                                // 24
  /* 78 */ 0x48,                                                // 36
  /* 79 */ 0x6c,                                                // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01,                                     // Channel set, length
  /* 82 */      0x01,                                           // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,

  //  0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,

  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};

void nextChannel() {
  if (sizeof(channels) < 2) {
    return;
  }

  uint8_t ch = channels[channelIndex];

  channelIndex++;
  if (channelIndex > sizeof(channels)) {
    channelIndex = 0;
  }

  if (ch != wifi_channel && ch >= 1 && ch <= 14) {
    wifi_channel = ch;

    // wifi_set_channel(wifi_channel);
    esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  }
}

void randomMac() {
  randomSeed(millis()); 
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
}

void payload_setup() {
  for (int i = 0; i < 32; i++) {
    emptySSID[i] = ' ';
  }
  randomSeed(1);
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31;

  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }
  randomMac();
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
  Serial.println("\n\nSSIDs:");
  int i = 0;
  int len = sizeof(ssids);
  while (i < len) {
    Serial.print((char) pgm_read_byte(ssids + i));
    i++;
  }

  Serial.println("\nExited cleanly..[OK]\n");
  Serial.println("\nRecon server down [OK]\n"); 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLUE); 
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(1,1);
  M5.Lcd.println("Panic Mode is Active!"); 
}

void payload_B() {
   currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // Alpha Numeric Random ssid pulse 28/s 
      memset(str, '\0', sizeof(str)); 
      uint8_t cnt = 0;
      while (cnt != sizeof(str) - 1) {
        str[cnt] = random(0, 0x7F);
      if (str[cnt] == 0) { break; }
      if (isAlphaNumeric(str[cnt]) == true) { cnt++; }
      else {
        str[cnt] = '\0';
      }
      memcpy(&beaconPacket[38], str, 32);
      memcpy_P(&beaconPacket[38], &str, 32);
      }
      beaconPacket[82] = wifi_channel;
      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

     i += j;
    }
  }

  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    cat_sideways_color();
    M5.Lcd.print(packetCounter); 
    Serial.print(packetCounter);
    Serial.println(" packets/s");
    cat_sideways_color();
    M5.Lcd.println(" packets/s"); 
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s \n[Playload B]");
    cat_sideways_color();
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.println("Panic Mode Active");   
    packetCounter = 0;
  } 
}


void payload() {
   currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);    
     
     // reset SSID 
      memcpy(&beaconPacket[38], emptySSID, 32);

      // write new SSID into beacon frame
      memcpy_P(&beaconPacket[38], &ssids[i], ssidLen);

      // set channel for beacon frame
      beaconPacket[82] = wifi_channel;

      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

      i += j;
    }
  }

  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    cat_sideways_color();
    M5.Lcd.print(packetCounter); 
    Serial.print(packetCounter);
    Serial.println(" packets/s");
    cat_sideways_color();
    M5.Lcd.println(" packets/s"); 
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s \n[Playload A]");
    cat_sideways_color();
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.println("Panic Mode Active");   
    packetCounter = 0;
  } 
}
/////////////END HACKING ZONE/////////////////

void setup() {  
Serial.begin(115200); // 115200 baud output.
M5.begin();
pinMode(A, INPUT); 
pinMode(B, INPUT); 
Serial.println(" ");
Serial.println("Serial active @ 115200 baud"); 
Serial.println("M5 Display [OK]"); 
delay(AX); 
Serial.println("Random number gen now seeding. [OK]");  
randomSeed(millis());
Serial.println("M5 Display in terminal mode..[OK]"); 
M5.Lcd.fillScreen(BLUE);
M5.Lcd.setTextColor(WHITE, BLUE); 
M5.Lcd.setRotation(3);
M5.Lcd.setCursor(1,1);
M5.Lcd.println("Starting..");
M5.Lcd.println("WiFi station Active, \nEssid:WhiteCat"); 
M5.Lcd.println("MAC ADDRESS:\n"+WiFi.macAddress()); 
Ethernet.begin(mac, local_IP, LocalServer, gateway, subnet);
WiFi.softAP(ssid, pass);
M5.Lcd.println("Password: whitecat1");
server.begin(); 
M5.Lcd.println("[Setup Finished]"); 
Serial.println("WiFi station Active, \nEssid:WhiteCat \nPassword:whitecat1"); 
Serial.print("Entering Delay loop.");
for(int i = 0; i <=2; i++) {  Serial.print("."); delay(XX+XX); }
M5.Lcd.fillScreen(BLACK);
Serial.println(".Finished delay loop [OK]"); 
img.setColorDepth(16); // Set color depth to 16
img.createSprite(IW,IH); // Create the sprite Width & Height  
img.fillSprite(WHITE); //
for(int pos = IW; pos > 0; pos--) {
build_banner("Hello I am WhiteCat ...",pos);
img.pushSprite(0,0);
button_press();
cat_normal(); 
}

img.deleteSprite(); // Free up memory 
M5.Lcd.fillScreen(BLACK);
Serial.print("Random Number output is:"); 
Serial.println(random(millis())); // debug random number output
int xarray[10]; 
for (int i=0; i<10; i++) xarray[i]= random(100);
Serial.println("WhiteCat network is online.");
Serial.println("Debuging Random number gen output:");
for (int i=0; i<11; i++) {
  Serial.print("Loop #"); 
  Serial.print(i); 
  Serial.print(" Random output ="); 
  Serial.println(xarray[i]);
  randomSeed(xarray[i]);
  } 

}


void loop() {
button_press(); 
// DO RANDOM CATS 
int randomNumber;
randomNumber = random(31);

Serial.print("Random Number = ");
Serial.print(randomNumber);
Serial.println(" "); 
 
switch(randomNumber) {  

case 1: button_press(); 
for (int i = 0; i <= random(255); i++) {
cat_sleep_normal2();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 2: button_press(); 
for (int i = 0; i <= random(203); i++) {
cat_run_fast();  
}
random_message();
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;
 
case 3: button_press(); 
for (int i = 0; i <= random(300); i++) {
cat_sleep_normal();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 4: button_press(); 
for (int i = 0; i <= random(100); i++) {
digipet();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 5: button_press(); 
for (int i = 0; i <= random(5); i++) {
cat_wink_left();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 6: button_press(); 
for (int i = 0; i <= random(200); i++) {
cat_alert_normal();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 7: button_press(); 
for (int i = 0; i <= random(155); i++) {
cat_wink_normal();  
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 8: button_press(); 
for (int i = 0; i <= random(11); i++) {
cat_wink_right();  
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 9: check_board(); 
for (int i = 0; i <= random(355); i++) {
cat_run();  
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
random_message();
break; 

case 10: button_press(); 
for (int i = 0; i <= random(160); i++) {
cat_lookingDown2();  
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 11:  button_press(); 
for (int i = 0; i <= random(160); i++) {
digipet();
cat_lookingDown2();  
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 12: check_board(); 
for (int i = 0; i <= random(111); i++) {
cat_diging();  
random_message();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 13: button_press(); 
for (int i = 0; i <= random(200); i++) {
  cat_wink_left();
  cat_wink_right();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 14: button_press(); 
for (int i = 0; i <= random(30); i++) { cat_normal(); }
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 15: button_press(); 
random_message(); 
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 16: button_press(); 
random_move(); 
for (int i = 0; i <= random(30); i++) {
random_move();
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 17: button_press(); 
for (int i = 0; i <= random(30); i++) {
  random_move_color(); 
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 18: button_press();
for (int i = 0; i <= random(20); i++) {
cat_normal3(); 
}
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 19: button_press(); 
for (int i = 0; i <= XX; i++) { check_board(); }
random_move_color(); 
break;

case 20: check_board(); 
for (int i = 0; i <= XX; i++) { check_board(); }
random_move_color(); 
random_message(); 
break; 

case 21: button_press(); 
for (int i = 0; i <= random(AX); i++) { check_board(); }
random_move_color(); 
cat_diging();
break; 

case 22: check_board(); 
for (int i = 0; i <= random(17); i++) {
  random_move_color2();
  random_move_color(); 
  random_move_color2();
} 
break;

case 23: check_board();
for (int i = 0; i <= random(34); i++) {
  cat_dig();
}
break; 

case 24: check_board(); 
for (int i=0; i == random(7); i++) {
cat_flash(); 
} 
break; 

case 25: check_board(); 
for (int i=0; i <= random(16); i++) {
  cat_sideways(); 
}
break; 

case 26:  check_board(); 
//int values have to be in for loop statment, 
//or you will Error out with Jump to case label ? 
for (int i=2; i < 30; i = i * 1.5) { 
  cat_sideways(); 
  } // logarithmic progression of loop
for (int i=40; i>=1; i--) {
  cat_sideways(); 
} // count down from random loop
break; 

case 27: check_board(); 
for (int i=0; i <= random(16); i++) {
cat_normal_flash(); 
}  
break; 

case 28: check_board(); 
for (int i=50; i>=1; i--) {
  cat_normal(); 
  cat_normal_flash(); 
}
button_press(); 
break;

case 29: button_press(); 
for (int i = 0; i <= XX; i++) { check_board(); }
for (int i = 0; i == 9; i++) { random_message(); } 
break; 

case 30: check_board(); 
for (int i = 0; i == 9; i++) { random_message(); }
break;

case 31: check_board(); 
for (int i=2; i < 30; i = i * 1.5) { 
  cat_sideways_color(); 
  } // logarithmic progression of loop
for (int i=40; i>=1; i--) {
  cat_sideways_color(); 
}
button_press(); 
break; 

default:  cat_normal(); 
          button_press(); 
break;

}
button_press(); 
}

void button_press() {
// If buttons long press do panic mode 
cur_val = digitalRead(A); 
if(cur_val != last_val) {
  if(cur_val==0) {
  payload_setup(); 
  for (int i = 0; i <= 50; i++) { payload(); }
  for (int i = 0; i <= 50; i++) { payload_B(); }    
  Serial.println("Restarting our recon server."); 
  M5.Lcd.fillScreen(BLACK); 
  server.stop(); 
  WiFi.softAP(ssid, pass);
  server.begin(); 
  Serial.println("Recon server active [OK]!"); 
    } else {
   Serial.println("..");  
  }
 last_val = cur_val;  
}
cur_val = digitalRead(B); 
if(cur_val != last_val) {
 if(cur_val==0) {
  payload_setup();
  for (int i = 0; i <= 50; i++) { payload_B(); }
  for (int i = 0; i <= 50; i++) { payload(); }
  M5.Lcd.fillScreen(BLACK);  
  Serial.println("Recon server restarting."); 
  server.stop(); 
  WiFi.softAP(ssid, pass);
  server.begin(); 
  Serial.println("Recon server active [OK]!"); 
  } else {
  Serial.println(".."); 
 }
 last_val = cur_val; 
}
}

void cat_normal() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void cat_normal_color() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, random(0xFFFF));
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void cat_diging() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_dig1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(211));
M5.Lcd.drawXBitmap(x, y, cat_dig1, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(211));
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_BLACK);  
}

void cat_lookingDown2() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_lookingDown, IW2, IH2, TFT_WHITE);
delay(WAIT+random(111));
M5.Lcd.drawXBitmap(x, y, cat_lookingDown, IW2, IH2, TFT_BLACK);
}

void cat_dig() {
int x = 30;
int y = 30;  
for (int i = 0; i <= 7; i++) {
M5.Lcd.drawXBitmap(x, y, cat_board, IW2, IH2, TFT_WHITE);
delay(WAIT+random(211));
M5.Lcd.drawXBitmap(x, y, cat_board, IW2, IH2, TFT_BLACK);  
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(111));
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_dig1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(111));
M5.Lcd.drawXBitmap(x, y, cat_dig1, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(111));
M5.Lcd.drawXBitmap(x, y, cat_dig2, IW2, IH2, TFT_BLACK);
}
}


void cat_normal_flash() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, random(0xFFFF));
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(random(40)); 
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, random(0xFFFF));
delay(random(50)); 
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void cat_sideways() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_sideways1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(233));
M5.Lcd.drawXBitmap(x, y, cat_sideways1, IW2, IH2, TFT_BLACK);
delay(WAIT+random(222)); 
M5.Lcd.drawXBitmap(x, y, cat_sideways2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(232));
M5.Lcd.drawXBitmap(x, y, cat_sideways2, IW2, IH2, TFT_BLACK);
delay(WAIT+random(229));
}

void cat_sideways_color() {
int x = random(IW);
int y = random(IH);
M5.Lcd.drawXBitmap(x, y, cat_sideways1, IW2, IH2, random(0xFFFF));
delay(WAIT+random(233));
M5.Lcd.drawXBitmap(x, y, cat_sideways1, IW2, IH2, TFT_BLACK);
delay(WAIT+random(222)); 
M5.Lcd.drawXBitmap(x, y, cat_sideways2, IW2, IH2, random(0xFFFF));
delay(WAIT+random(232));
M5.Lcd.drawXBitmap(x, y, cat_sideways2, IW2, IH2, TFT_BLACK);
delay(WAIT+random(229));
}

void cat_flash() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, random(0xFFFF));
delay(WAIT+AX);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void random_move() {
for (int i = 0; i <= 10; i++) {  
int x = random(IW); 
int y = random(IH); 
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);  
} 
}

void random_move_color() {
for (int i = 0; i <= 10; i++) {  
int x = random(IW); 
int y = random(IH); 
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, random(0xFFFF));
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, random(0xFFFF));
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, random(0xFFFF));
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);  
check_board();  
} 
}

void random_move_color2() {
  for (int i = 0; i <= 18; i++) {  
int x = random(IW)+random(1); 
int y = random(IH)+random(2); 
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(y, x, cat_run1, IW2, IH2, random(0xFFFF));
delay(random(100));
M5.Lcd.drawXBitmap(y, x, cat_run1, IW2, IH2, TFT_BLACK); 
delay(random(90)); 
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(y, x, cat_run2, IW2, IH2, random(0xFFFF));
delay(random(90));
M5.Lcd.drawXBitmap(y, x, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(y, x, cat_run3, IW2, IH2, random(0xFFFF));
delay(random(99));
M5.Lcd.drawXBitmap(y, x, cat_run3, IW2, IH2, TFT_BLACK);  
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_WHITE);
delay(random(100));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);  
} 
}

void cat_normal2() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}
void cat_normal3() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, CRT(256));
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void cat_wink_right() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_WHITE);
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
}

void cat_wink_left() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wink2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wink2, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
}

void cat_wink_normal() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_WHITE);
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_wink2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(156));
M5.Lcd.drawXBitmap(x, y, cat_wink2, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
}

void digipet() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake1, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake2, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void cat_sleep_normal() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_sleep, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_sleep, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_sleep2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_sleep2, IW2, IH2, TFT_BLACK);
}

void cat_run_fast() {
for (int i = 0; i <= 5; i++) {  
int x = 30;
int y = 30+i;
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_WHITE);
delay(random(191));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_WHITE);
delay(random(192));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_WHITE);
delay(random(193));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);  
}
}

void cat_run() {
for (int i = 0; i <= 6; i++) {
int x = 30+i;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(350));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(250));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_WHITE);
delay(WAIT+random(350));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_WHITE);
delay(WAIT+random(350));
M5.Lcd.drawXBitmap(x, y, cat_run1, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_WHITE);
delay(WAIT+random(350));
M5.Lcd.drawXBitmap(x, y, cat_run2, IW2, IH2, TFT_BLACK);
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_WHITE);
delay(WAIT+random(350));
M5.Lcd.drawXBitmap(x, y, cat_run3, IW2, IH2, TFT_BLACK);
}
}

void cat_sleep_normal2() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(650));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_WHITE);
delay(WAIT+random(650));
M5.Lcd.drawXBitmap(x, y, cat_wink, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_board, IW2, IH2, TFT_WHITE);
delay(WAIT+random(550));
M5.Lcd.drawXBitmap(x, y, cat_board, IW2, IH2, TFT_BLACK);
}

void cat_alert_normal() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_alert, IW2, IH2, TFT_WHITE);
delay(WAIT+random(150));
M5.Lcd.drawXBitmap(x, y, cat_alert, IW2, IH2, TFT_BLACK);
}


void banner_noback(String msg, int xpos) {
  int h = IH;
  img.setTextSize(2);
  // Note: make sure to set this outside our loops,
  // or it will flash random wild colors to our text. 
  // img.setTextColor(random(0xFFFF)); // set random color.
  img.setCursor(xpos, 1);
  img.setTextFont(2);    // Options number
  img.setTextWrap(false);  
  img.setCursor(xpos - IW, 2);
  while (h--) img.drawFastHLine(0, h, IW, BLACK);
  img.print(msg);
}

void random_message() {
for (int i = 0; i <= random(AX); i++) { check_board(); }
M5.Lcd.fillScreen(BLACK);

img.createSprite(IW,IH); // Create the sprite Width & Height  
img.fillSprite(BLACK); // fill our sprite background as black.  

int i = random(26); 

Serial.print("Random Number = ");
Serial.print(i);
Serial.println(" "); 

switch (i) {

case 0: button_press(); 
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("I solemnly swear that...",pos);
img.pushSprite(0,0);
cat_normal(); 
}
for(int pos = IW; pos > 0; pos--) {
  banner_noback("I am up to no good!",pos);
  img.pushSprite(0,0);
  cat_normal();
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 1: button_press(); 
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("You can't make everyone happy.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 2: button_press();
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("Do not take life too seriously.. ",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 3: button_press(); 
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos <= 0; pos--) {
banner_noback("if you want the rainbow, deal with rain.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 4: button_press();
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("When nothing is going right, go fish",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 5: button_press(); 
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("Remember to take it easy..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 6: button_press();
img.setTextColor(random(0xFFFF)); // set random color.
for(int pos = IW; pos > 0; pos--) {
banner_noback("I could agree with you, ",pos);
img.pushSprite(0,0);
cat_normal(); 
}
for(int pos = IW; pos > 0; pos--) {
banner_noback("but then we'd both be wrong.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 7: button_press(); 
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("I am a liability!",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 8: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 9: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Evil grows in the dark!",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 10: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Meeow! Meeow!",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 11: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Remember to look up...",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 12: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("All good things are wild.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 13: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("FREE as in freedom....",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 14: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Joy is within..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 15: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Work hard; Play hard..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 16: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Sometimes BEER is a good thing.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 17: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("It's wine time bro.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 18: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Give it 100% today..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 19: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Coffee break already! ",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 20: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Fuck around & find out...",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 21: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("I will rock your socks off..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 22: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Caffeine, caffeine, caffeine!",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 23: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("I am down with the underground.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 24: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Don't be evil? don't be google.",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

case 25: button_press();
img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("I'm not a watch..",pos);
img.pushSprite(0,0);
cat_normal(); 
}
for(int pos = IW; pos > 0; pos--) {
banner_noback("IDEA: finger sized computer?",pos);    
cat_wink_right(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 

case 26: button_press();
 img.setTextColor(random(0xFFFF)); // set random color. 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Searching networks..",pos);
img.pushSprite(0,0);
cat_normal_color(); 
}
for(int pos = IW; pos > 0; pos--) {
banner_noback("Hardware proxy is active.",pos);    
cat_normal_color(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break; 
default: 
for(int pos = IW; pos > 0; pos--) {
banner_noback("Q:A tree's final moment of revenge?",pos);
img.pushSprite(0,0);
cat_normal(); 
}
for(int pos = IW; pos > 0; pos--) {
banner_noback("A: Papercut!",pos);
img.pushSprite(0,0);
cat_normal(); 
}
img.deleteSprite();  
M5.Lcd.fillScreen(BLACK);
for (int i = 0; i <= random(AX); i++) { check_board(); }
break;

}
}

void build_banner(String msg, int xpos) {
  int h = IH;
  while (h--) img.drawFastHLine(0, h, IW, RGBVAL(h * 4.99));
  img.drawRect(IW - 1, IH - 1, 40, 320, WHITE);
  img.setTextSize(1);
  img.setTextColor(BLACK);
  img.setCursor(xpos, 1);
  img.setTextFont(4);    // Options number
  img.setTextWrap(false); // Buggy if set to true - Options true / false 
  img.setCursor(xpos - IW, 2);
  img.print(msg); 
  button_press();
}
