/*
   MIT License

  Copyright (c) 2021 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <ESP32Time.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "time.h"
#include "esp_sntp.h"
#include <SPI.h>
#include <Arduino.h>

//ESP32Time rtc;
ESP32Time rtc(0);  // offset in seconds GMT+1


uint8_t rst = 16;
uint8_t d_clk = 17;
uint8_t o_clk = 18;
uint8_t nixi_en = 8;
uint8_t nixi_data = 19;
uint8_t currentHour;
uint8_t currentMin;

uint16_t year = 2023;
uint8_t month = 9;
uint8_t day = 7;
uint8_t hour = 11;
uint8_t minute = 24;
uint8_t second = 30;
uint32_t lastMillis;



const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


void setup() {
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    



    pinMode(rst, OUTPUT);
    pinMode(d_clk, OUTPUT);
    pinMode(o_clk, OUTPUT);
    pinMode(nixi_en, OUTPUT);
    pinMode(nixi_data, OUTPUT);
    digitalWrite(rst,1);
    digitalWrite(nixi_en, 0);
    struct tm timeinfo;
    currentHour = timeinfo.tm_hour;
    currentMin = timeinfo.tm_min;
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
     res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
        printLocalTime();

    }


}




void printLocalTime() 
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
   
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;

  //rtc.setDateTime(year, month, day, hour, minute, second);
  rtc.setTime(second, minute, hour, day, month, year);  // 17th Jan 2021 15:24:30

  
  

}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) 
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void sendToRegister(uint8_t bit)
{
  if (bit == 1)
  {
    digitalWrite(nixi_data, 1);
  } else {
    digitalWrite(nixi_data, 0);
  }

  delay(1);
  digitalWrite(d_clk, 1);
  delay(1);
  digitalWrite(o_clk,1);
  delay(1);
  digitalWrite(d_clk,0);
  delay(1);
  digitalWrite(o_clk,0);
  delay(1);
}

void sendNumber(uint8_t number)
{
  switch(number)
  {
    case 0:
      digitalWrite(nixi_en, 1);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 0");
      break;
    case 1:
      digitalWrite(nixi_en, 1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 1");
      
      break;
    case 2:
      digitalWrite(nixi_en, 1);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 2");
      break;
    case 3:
      digitalWrite(nixi_en, 1);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 3");
      break;
    case 4:
      digitalWrite(nixi_en, 1);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 4");
      break;
    case 5:
      digitalWrite(nixi_en, 1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 5");
      break;
    case 6:
      digitalWrite(nixi_en, 1);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 6");
      break;
    case 7:
      digitalWrite(nixi_en, 1);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 7");
      break;
    case 8:
      digitalWrite(nixi_en, 1);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 8");
      break;
    case 9:
      digitalWrite(nixi_en, 1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      digitalWrite(nixi_en, 0);
      Serial.println("printing 9");
      break;           
  }
}

void loop() 
{
    // put your main code here, to run repeatedly:   
    //printLocalTime();


    if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        minute = rtc.getMinute();
        hour = rtc.getHour();
        second = rtc.getSecond();
        day = rtc.getDay();
        month = rtc.getMonth();
        year = rtc.getYear();
        if(currentHour != hour  || currentMin != minute)
          {
            sendNumber(minute%10);
            sendNumber(minute/10);
            sendNumber(hour%10);
            sendNumber(hour/10);
            currentHour = rtc.getHour();
            currentMin = rtc.getMinute();
            Serial.println("new local time");
          } else {
            Serial.println("still same time");
          }
    }

   //  Serial.println(rtc.getLocalEpoch());         //  (long)    1609459200 epoch without offset
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));   // (String) returns time with specified format 
  delay(1000);
}
