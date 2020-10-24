/* DATA -> GPIO13 -> D7
 *  SRCLK -> GPIO15 -> D8
 *  RCLK -> GPIO14 -> D5
 *  DIGIT 2_4 -> GPIO4 -> D2
 *  DIGIT 1 -> GPIO5 -> D2
 *  LED_PIN -> GPIO2 (?) -> D4
 */

#include "Arduino.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char *ssid     = "WiFi de Ronan";
const char *password = "XXXX";

const long utcOffsetInSeconds = 2*3600;

#define DATA_PIN    13
#define SRCLK_PIN   5
#define RCLK_PIN    14 /* LATCH*/
#define DIGIT_2_4   16
#define DIGIT_1     4
#define DOTS        12


byte i = 0;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  pinMode(DATA_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(DIGIT_2_4, OUTPUT);
  pinMode(DIGIT_1, OUTPUT);
  pinMode(DOTS, OUTPUT);
  
  digitalWrite(DIGIT_2_4, HIGH);
  digitalWrite(DIGIT_1, HIGH);
  digitalWrite(DOTS, HIGH);
  //Serial.begin(115200);

   Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
}

// the loop function runs over and over again forever
void loop()
{
  ArduinoOTA.handle();
  timeClient.update();
  int dotStatus = digitalRead(DOTS);
  digitalWrite(DOTS, dotStatus == LOW ? HIGH : LOW);

  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  
  digitalWrite(RCLK_PIN, LOW);

  // Send minutes first :
  byte minutesLow = ((byte)((timeClient.getMinutes() / 10)));
  if(minutesLow == 0){
    minutesLow = 9;
  }
  else
    minutesLow--;
  byte minutesHigh = ((((timeClient.getMinutes() % 10))));
  if(minutesHigh == 0){
    minutesHigh = 9;
  }
  else
    minutesHigh--;
  byte minutes = minutesLow | (minutesHigh <<4);
  Serial.println(minutes, HEX);
  shiftOut(DATA_PIN, SRCLK_PIN, minutes);
  byte hoursLow = ((byte)((timeClient.getHours() / 10)));
  if(hoursLow == 0){
    hoursLow = 9;
  }
  else
    hoursLow--;
  byte hoursHigh = ((((timeClient.getHours() % 10) )));
  if(hoursHigh == 0){
    hoursHigh = 9;
  }
  else
    hoursHigh--;

  byte hours = hoursLow | (hoursHigh) << 4;

  shiftOut(DATA_PIN, SRCLK_PIN, hours);

  

  digitalWrite(RCLK_PIN, HIGH);

  delay(1000);              // Wait for two seconds (to demonstrate the active low LED)
}

void shiftOut(int myDataPin, int myClockPin, byte myDataOut)
{
  int i = 0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);
  for (i = 7; i >= 0; i--)
  {
    digitalWrite(myClockPin, 0);
    if (myDataOut & (1 << i))
    {
      pinState = 1;
    }
    else
    {
      pinState = 0;
    }
    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }
  //stop shifting
  digitalWrite(myClockPin, 0);
}
