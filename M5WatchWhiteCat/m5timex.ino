//  
// Project: Buster the Watch cat. 
// Sketch filename: m5timex
// Includes filename: include_def.h
// Database filename: xmaps.h
// 
// Please note this clock is 24 hour standard time.
//
// NTP server http://0.north-america.pool.ntp.org/   
// https://www.ntppool.org/en/  
// pool.ntp.org: public ntp time server for everyone.
//  NTP server urls: 
// http://0.north-america.pool.ntp.org/   
// https://www.ntppool.org/en/  
//
// Timezone urls: 
// https://www.timeanddate.com/  
// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
// So with much reading I found the following to be effective daylight offset in Sec = -7  (timezone maps are available on-line)  GMT offset = -25200       
// https://en.wikipedia.org/wiki/List_of_UTC_time_offsets
// pool.ntp.org: public ntp time server for everyone.
// NTP is an internet protocol that’s used to synchronise the clocks on computer networks to within a few milliseconds of universal coordinated time (UTC). 
// It enables devices to request and receive UTC from a server that, in turn, receives precise time from an atomic clock.
// The pool.ntp.org project is a big virtual cluster of timeservers providing reliable easy to use NTP service for millions of clients.
// The pool is being used by hundreds of millions of systems around the world. 
// It's the default "time server" for most of the major Linux distributions and many networked appliances such as smart devices / internet of things etc.  
// With a little luck and searching you will find the correct settings for your timezone.  
//
// How do I use the timezone map and find the GMT offset in milliseconds?
// You need to adjust the UTC offset for your timezone in milliseconds. 
// You can do so with this simple math trick:
// UTC -7*60*60 = −25200
// 
// Daylight saving time? 
// The website timeanddate.com is awesome for finding the current UTC/GMT offset.
// And you can use this website to search by state anywhere in the world for timezone offsets.  
// UTC -6*60*60 = -21600 
// 
// Version codename: Watch cat 
//
/////////////////////

#include <M5StickC.h>
#include <WiFi.h>
#include "time.h"
#include "include_def.h"
#include "xmaps.h"

const char* ntpServer = "0.north-america.pool.ntp.org"; 
const long  gmtOffset_sec = -25200; 
const int daylightOffset_sec = -25200; //-7;
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

void printLocalTime() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  M5.Lcd.setCursor(1,1); 
  M5.Lcd.setTextColor(yellow); 
  M5.Lcd.println(&timeinfo, "%A %B %Y\n"); 
  M5.Lcd.println(&timeinfo, "DATE %D\n");
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(random(0xFFFF));  
  M5.Lcd.print(&timeinfo, "%H");
  M5.Lcd.print(":"); 
  M5.Lcd.print(&timeinfo, "%M"); 
  M5.Lcd.print(".");  
  M5.Lcd.println(&timeinfo,"%S\n"); 
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = timeinfo.tm_hour;
    TimeStruct.Minutes = timeinfo.tm_min;
    TimeStruct.Seconds = timeinfo.tm_sec;
    M5.Rtc.SetTime(&TimeStruct);
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = timeinfo.tm_wday;
    DateStruct.Month = timeinfo.tm_mon + 1;
    DateStruct.Date = timeinfo.tm_mday;
    DateStruct.Year = timeinfo.tm_year + 1900;
    M5.Rtc.SetData(&DateStruct);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 55);
  M5.Lcd.setTextColor(orange); 
  M5.Lcd.printf("\n[ ONLINE ] WiFi MODE HOME"); 
  delay(WAIT);    
}

void setup() {
M5.begin(); 
M5.Lcd.setRotation(3); 
M5.Lcd.setTextColor(random(0xff80)); 
WiFi.begin(ssid, password); 
while(WiFi.status() != WL_CONNECTED) {
  M5.Lcd.setCursor(0, 0);
   static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);
  M5.Lcd.setTextColor(yellow); 
  M5.Lcd.printf("DATE: %04d-%02d-%02d(%s)\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 12);
  M5.Lcd.setTextColor(random(0xFF80));  
  M5.Lcd.printf("%02d:%02d.%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.setTextColor(gray);  
  M5.Lcd.printf("\n[ OFFLINE ] RTC MODE"); 
  delay(1350);
  M5.Lcd.fillScreen(BLACK);  
  logic_gate(); 
} 
  delay(WAIT); 
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setCursor(0, 0);
 

configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
printLocalTime();
WiFi.disconnect(true); 
WiFi.mode(WIFI_OFF); 
delay(WAIT); 
}

void loop() {
for (int q = 0; q <= 20; q++) {  
M5.Lcd.setTextColor(random(0xFFFF)); 
printLocalTime(); 
delay(WAIT);
M5.Lcd.fillScreen(BLACK); 
}
logic_gate(); 
}

int logic_gate() {
int randomNumber;
randomNumber = random(33);

switch(randomNumber) {  
case 0: cat_normal(); 
        break;
case 1: cat_normal_color();
        break;
case 2: cat_diging();
        break;
case 3: cat_lookingDown2();
        break;
case 4: cat_dig(); 
        break;
case 5: cat_normal_flash();
        break;
case 6: cat_sideways(); 
        break;
case 7: cat_sideways_color();
        break;
case 8: cat_flash();
        break;      
case 9: random_move(); 
        break;     
case 10: random_move_color(); 
        break;
case 11: random_move_color2(); 
        break;
case 12: cat_normal2();
        break; 
case 13: cat_normal3();
        break;         
case 14: cat_wink_right(); 
        break;
case 15: cat_wink_left(); 
        break;
case 16: cat_wink_normal(); 
        break;
case 17: digipet(); 
        break;  
case 18: cat_sleep_normal(); 
        break;
case 19: cat_run_fast(); 
        break;      
case 20: cat_run(); 
        break; 
case 21: cat_sleep_normal2();
        break;
case 22: cat_sleep_normal_short(); 
        break; 
case 23: cat_alert_normal();
        break;
case 24: cat_flash(); 
         cat_sideways(); 
         break;
case 25: cat_dig();
         cat_sideways(); 
         break;    
case 26: cat_run_fast();
         break;
case 27: cat_normal_flash(); 
         break;   
case 28: random_move_color();
         random_move_color2(); 
         break;
case 29: cat_normal(); 
         cat_normal_flash(); 
         break;
case 30: cat_normal(); 
         random_move_color2(); 
         break;
case 31: tailwag();
         break; 
case 32: tailwag_color(); 
         break;  
case 33: tailwag();
         tailwag_color(); 
         break;                                                                                                                        
default: cat_normal(); 
         break;
}

}

unsigned int CRT(byte value) { 
byte red_blue_green = random(0xFFFF);
return red_blue_green * value;  
}

void cat_normal() {
int x = 30;
int y = 30;
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}

void tailwag() {
int x = 30; 
int y = 30;
for (int i = 0; i <= 10; i++) {
M5.Lcd.drawXBitmap(x, y, cat_tailwag_down, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_tailwag_down, IW2, IH2, TFT_BLACK);
delay(WAIT); 
M5.Lcd.drawXBitmap(x, y, cat_tailwag_up, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_tailwag_up, IW2, IH2, TFT_BLACK);
}
}

void tailwag_color() {
int x = 30; 
int y = 30;
for (int i = 0; i <= 10; i++) {
M5.Lcd.drawXBitmap(x, y, cat_tailwag_down, IW2, IH2, CRT(100));
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_tailwag_down, IW2, IH2, TFT_BLACK);
delay(WAIT); 
M5.Lcd.drawXBitmap(x, y, cat_tailwag_up, IW2, IH2, CRT(100));
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_tailwag_up, IW2, IH2, TFT_BLACK);
}
}



void cat_normal_color() {
int x = 30;
int y = 30;
for (int i = 0; i <= 10; i++) {
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, random(0xFFFF));
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_wake, IW2, IH2, TFT_BLACK);
}
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

void cat_sleep_normal_short() {
int x = 30; 
int y = 30; 
M5.Lcd.drawXBitmap(x, y, cat_sleep2, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_sleep2, IW2, IH2, TFT_BLACK); 
M5.Lcd.drawXBitmap(x, y, cat_sleep, IW2, IH2, TFT_WHITE);
delay(WAIT);
M5.Lcd.drawXBitmap(x, y, cat_sleep, IW2, IH2, TFT_BLACK); 
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
