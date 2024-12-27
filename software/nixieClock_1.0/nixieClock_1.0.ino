/*


  MIT License

  Copyright (c) 2021 Jakub Sanecki

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



  @author Jakub Sanecki
  @version 1.0
  @date 2024-12-25

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


uint32_t lastMillis;
uint16_t year = 2023;
uint8_t month, day, hour, minute, second, currentHour, currentMin;



const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


void setup() 
{
    

    Serial.begin(115200);

    // initialize the nixie tube lapmps
    pinMode(rst, OUTPUT);
    pinMode(d_clk, OUTPUT);
    pinMode(o_clk, OUTPUT);
    pinMode(nixi_en, OUTPUT);
    pinMode(nixi_data, OUTPUT);

    // set the initial state of the nixie clock
    digitalWrite(rst,1);
    digitalWrite(nixi_en, 0);

    // create an instance of the ESP32Time class
    struct tm timeinfo;
    currentHour = timeinfo.tm_hour;
    currentMin = timeinfo.tm_min;

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    

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

        // set the time zone
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

        //seting time 
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
  
  //downloading time from NTP
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;

  // set the time on the rtc
  rtc.setTime(second, minute, hour, day, month, year);  // 17th Jan 2021 15:24:30

}


// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) 
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}


// Function to send data to the nixie tube
void sendToRegister(uint8_t bit)
{
  if (bit == 1)
  {
    digitalWrite(nixi_data, 1);
  } else {
    digitalWrite(nixi_data, 0);
  }

  digitalWrite(d_clk, 1);
  digitalWrite(o_clk,1);
  digitalWrite(d_clk,0);
  digitalWrite(o_clk,0);
}


// Decodes the number to be displayed on the nixie tube
void sendNumber(uint8_t number)
{
  Serial.println("Sending number: " + String(number));
  switch(number)
  {
    case 0:
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      break;
    case 1:
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);    
      break;
    case 2:
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      break;
    case 3:
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      break;
    case 4:
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(0);
      break;
    case 5:
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      break;
    case 6:
      sendToRegister(1);
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(0);
      break;
    case 7:
      sendToRegister(0);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      break;
    case 8:
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(1);
      sendToRegister(0);
      break;
    case 9:
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(0);
      sendToRegister(1);
      break;           
  }
}


void loop() 
{

    if (millis() - lastMillis > 1000) {
        //getting time from RTC
        lastMillis = millis();
        //Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S")); // prints the time in the format "Monday, January 01 2021 00:00:00"
        hour = rtc.getTime("%H").toInt();
        minute = rtc.getTime("%M").toInt();
        day = rtc.getTime("%d").toInt();
        month = rtc.getTime("%m").toInt();


        if(currentHour != hour  || currentMin != minute) // check if the time has changed
          {
            if(currentMin%5 == 0) // check if the time is a multiple of 5
            {
              //Serial.println("showing date");
              sendNumber(month%10);
              sendNumber(month/10);
              sendNumber(day%10);
              sendNumber(day/10);
              delay(5000);
            }
            // send the time to the nixie tube
            //Serial.println("new local time");
            sendNumber(minute%10);
            sendNumber(minute/10);
            sendNumber(hour%10);
            sendNumber(hour/10);
            currentHour = hour;
            currentMin = minute;
          } else {
            //Serial.println("still same time");
          }
    }
}
