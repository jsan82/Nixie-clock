#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"


uint8_t rst = 16;
uint8_t d_clk = 17;
uint8_t o_clk = 18;
uint8_t nixi_en = 8;
uint8_t nixi_data = 19;
uint8_t rtc_int = 21;
uint8_t rtc_scl = 47;
uint8_t rtc_sda = 48;
uint8_t currentHour;
uint8_t currentMin;

const char *ssid = "HotPot";
const char *password = "ooYie3Ka";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
   
  int hour = timeinfo.tm_hour;
  int min = timeinfo.tm_min;
  
  if(currentHour != hour  || currentMin != min)
  {
    sendNumber(min%10);
    sendNumber(min/10);
    sendNumber(hour%10);
    sendNumber(hour/10);
    currentHour = timeinfo.tm_hour;
    currentMin = timeinfo.tm_min;
    Serial.println("new local time");
  } else {
    Serial.println("still same time");
  }

}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
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

void setup() {
  Serial.begin(115200);
  Serial.println("Starting");
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

  // First step is to configure WiFi STA and connect in order to get the current time and date.
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  /**
   * NTP server address could be acquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 acquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE acquired NTP server address
   */
  esp_sntp_servermode_dhcp(1);  // (optional)

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);

  /**
   * This will set configured ntp servers and constant TimeZone/daylightOffset
   * should be OK if your time zone does not need to adjust daylightOffset twice a year,
   * in such a case time adjustment won't be handled automagically.
   */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  /**
   * A more convenient approach to handle TimeZones with daylightOffset
   * would be to specify a environment variable with TimeZone definition including daylight adjustmnet rules.
   * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
   */
  //configTzTime(time_zone, ntpServer1, ntpServer2);
}

void loop() {
  delay(5000);
  printLocalTime();
  // for(int i = 0; i <= 9; i++)
  // {
  //   digitalWrite(rst,0);
  //   digitalWrite(rst,1);
  //   sendNumber(i);
  //   sendNumber(i);
  //   sendNumber(i);
  //   sendNumber(i);
  //   delay(10000);
  // }

}
