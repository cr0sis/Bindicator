

/*
   Bindicator by cr0sis 2020 all rights reserved.
   Got the copyright police on it an' all.
   Be cool to each other.*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define NUM_LEDS 85
#define DATA_PIN 1
CRGB leds[NUM_LEDS];

/* Set these to your desired credentials. */
const char *ssid = "********";  //ENTER YOUR WIFI SETTINGS
const char *password = "********";

// How many hours ahead of time to trigger notification
const long hoursBefore = 24;

unsigned long previousMillis = 0;
const long interval = 7200000;

//Web/Server address to read from
const char *host = "ilancasterapi.lancaster.ac.uk"; // Sorry, this is going to take some detective work on your part to put the right url in for your local API.
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80
//SHA1 finger print of certificate
const char fingerprint[] PROGMEM = "F2 F4 7C 52 1F 55 DD 06 6B E6 A0 6D E4 D4 6C 17 83 5F BC DF";
String link = "//production/api/v1/CityAndRegion/Bins/Collection/YOUR%20ADDRESS%20HERE%20"; // Sorry, this is going to take some detective work on your part to put the right url in for your house.


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60*60*hoursBefore); // tweak hoursBefore in globals above, this is hours before midnight on the collection day you want to start being informed. 24 = midnight the night before. 

//Week Days
const char *weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
const char *months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

constexpr size_t capacity = 4 * JSON_ARRAY_SIZE(1) + 5 * JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 9 * JSON_OBJECT_SIZE(2) + 560;
void setup() {
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(9600);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  FastLED.showColor(CRGB::Green);
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  //timeClient.setTimeOffset(3600);
  greenFade();
  fullUpdate();
}
void greenBin() {              // TIME TO TAKE OUT THE GREEN TRASH!
  for (int i = 0; i < 60; i++) {    //Flash the LED if it's Bin day!
    greenRise();
    delay(50);
    greenFade();
  }
}

void greenRise() {
  for (int u = 0; u <= 255; u++) {
    fill_solid(leds, NUM_LEDS, CHSV(96, 255, u));
    FastLED.show();
    delay(2);
  }
}

void greenFade() {
  for (int u = 255; u >= 0; u--) {
    fill_solid(leds, NUM_LEDS, CHSV(96, 255, u));
    FastLED.show();
    delay(2);
  }
}


void knightRider2() {
  for (int t = 0; t < 250; t++) {
  }
  for (int i = 0; i < NUM_LEDS - 1; i++) {
    leds[i] = CRGB::Purple;
    leds[i + 1] = CRGB::Purple;
    FastLED.delay(1);
    leds[i] = CRGB::Black;
    leds[i + 1] = CRGB::Black;
  }
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void rainbow() { 
  static uint8_t hue = 0;
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 125);
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }


  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 125);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

/*for(int i = 0; i < NUM_LEDS-1; i++) {
  leds(i,i+1) = CRGB::Red;
  FastLED.delay(33);
  leds(i,i+1) = CRGB::Black;
  }*/

/*void knightRider() {          // Knightrider

    for (int k = 0; k < NUM_LEDS; k++) {
      leds[k] = CRGB::Red;
      FastLED.delay(5.5);
      leds[k] = CRGB::Black;
    }
    for (int k = NUM_LEDS - 1; k >= 0; k--) {
      leds[k] = CRGB::Red;
      FastLED.delay(5.6);
      leds[k] = CRGB::Black;
    }
  }*/

void updating() {
  for (int u = 255; u >= 0; u--) {
    fill_solid(leds, NUM_LEDS, CHSV(181, 255, u));
    FastLED.show();
    delay(2);
  }
}

String getJson() {

  updating();

  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  //  Serial.println(host);
  updating();
  //  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  updating();
  Serial.println("Running new binformation update");
  int r = 0; //retry counter
  while ((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
    delay(100);
    Serial.println(".");
    r++;
  }
  if (r == 30) {
    Serial.println("Connection failed. ");
    return "";
  }
  else {
    Serial.println("Connected to web. ");
  }

  Serial.println("Requesting binformation");
  //  Serial.println(host + link);
  updating();
  httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  // Serial.println("request sent");
  updating();
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      updating();
      //     Serial.println("headers received");
      break;
    }
  }

  // Serial.println("reply was:");
  // Serial.println("==========");
  String line;
  while (httpsClient.available()) {
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    //   Serial.println(line); //Print response
  }
  //  Serial.println("==========");
  Serial.println("Binformation received");
  updating();
  String json = line;

  return json;
}

void fullUpdate() {
  DynamicJsonDocument doc(capacity);
  String json = getJson();
  String  collectionDate = "";

  if (json == "") {
    // THINK ABOUT notifying user of error in fetching data, e.g. by turning on a RED LED somewhere in future.
    return;
  }

  deserializeJson(doc, json);

  String nextCollection = doc[0]["CollectionDate"]; // "2020-05-01T00:00:00Z"
  String binType = doc[0]["BinTypes"][0];
  String nextCollection_1 = doc[1]["CollectionDate"]; // "2020-05-09T00:00:00Z"
  String binType_1 = doc[1]["BinTypes"][0]; // "Recycle"
  collectionDate = nextCollection.substring(0, nextCollection.length() - 10);

  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  //Serial.print("Epoch Time: ");
  //Serial.println(epochTime);

  String formattedTime = timeClient.getFormattedTime();
  // Serial.print("Formatted Time: ");
  Serial.println(formattedTime);

  struct tm *ptm = gmtime ((time_t *)&epochTime);

  String weekDay = weekDays[timeClient.getDay()];
  // Serial.print("Weekday: ");
  // Serial.println(weekDay);

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;

  String currentMonthStr = String(currentMonth);
  String monthDayStr = String(monthDay);

  if (currentMonthStr.length() == 1) {
    currentMonthStr = "0" + currentMonthStr;
  }
  if (monthDayStr.length() == 1) {
    monthDayStr = "0" + monthDayStr;
  }

  // Serial.print("Month day: ");
  // Serial.println(monthDayStr);

  // Serial.print("Month: ");
  // Serial.println(currentMonthStr);

  // String currentMonthName = months[currentMonth - 1];
  // Serial.print("Month name: ");
  // Serial.println(currentMonthName);

  int currentYear = ptm->tm_year + 1900;
  // Serial.print("Year: ");
  // Serial.println(currentYear);


  String currentDate = String(currentYear) + "-" + currentMonthStr + "-" + monthDayStr;
  Serial.print("Tomorrow's date: ");
  Serial.println(currentDate);
  //  Serial.println("==========");

  if ((currentDate == collectionDate) && (binType == "Green")) {
    Serial.print("It's now time to take the ");
    Serial.print(binType);
    Serial.print(" bin out!!!");
    greenBin();
    Serial.print("Your next collection date is: ");
    Serial.println(nextCollection_1);
    Serial.print("Next Bin Type: ");
    Serial.println(binType_1);
    Serial.print("");
  }
  if ((currentDate == collectionDate) && (binType == "Refuse")) {
    Serial.print("It's now time to take the ");
    Serial.print(binType);
    Serial.print(" bin out!!!");
    knightRider2();
    Serial.print("Your next collection date is: ");
    Serial.println(nextCollection_1);
    Serial.print("Next Bin Type: ");
    Serial.println(binType_1);
    Serial.print("");
  }
  else {
    Serial.print("Your next collection date is: ");
    Serial.println(collectionDate);
    Serial.print("Next Bin Type: ");
    Serial.println(binType);    
    Serial.print("");
  }
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you ran a Full Update
    previousMillis = currentMillis;
    fullUpdate();
  }
  else {
    rainbow();
  }
}


//=======================================================================

