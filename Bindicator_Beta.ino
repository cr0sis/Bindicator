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

#define NUM_LEDS 10
#define DATA_PIN 1
CRGB leds[NUM_LEDS];

/* Set these to your desired credentials. */
const char *ssid = "********";  //ENTER YOUR WIFI SETTINGS
const char *password = "********";


unsigned long previousMillis = 900000;
const long interval = 900000;

//Web/Server address to read from
const char *host = "ilancasterapi.lancaster.ac.uk";      // EXAMPLE URL ONLY - First part (Host only here) 
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80   // see below for where to put the rest of the url. 
                                                         // Search whole doc for "EXAMPLE" to toggle between these two lines.

//SHA1 finger print of certificate
const char fingerprint[] PROGMEM = "F2 F4 7C 52 1F 55 DD 06 6B E6 A0 6D E4 D4 6C 17 83 5F BC DF";


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

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
  timeClient.setTimeOffset(3600);
  greenFade();
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

void knightRider() {          // Knightrider
  //  for (int i = 0; i < 2; i++) {
  for (int k = 0; k < NUM_LEDS; k = k + 1) {
    leds[k] = CRGB::Red;
    FastLED.show();
    leds[k] = CRGB::Black;
    delay(50);
  }
  for (int k = (NUM_LEDS) - 1; k >= 0; k--) {
    leds[k] = CRGB::Red;
    FastLED.show();
    leds[k] = CRGB::Black;
    delay(50);
  }
}


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

  Serial.println(host);
  updating();
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  updating();
  Serial.println("HTTPS Connecting");
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

  String Link;

  //GET Data
  Link = "//production/api/v1/CityAndRegion/Bins/Collection/Your%20Address%URL"; // EXAMPLE URL ONLY - YOU WILL NEED TO FIND YOURS.

  Serial.println("requesting URL: ");
  Serial.println(host + Link);
  updating();
  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  Serial.println("request sent");
  updating();
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      updating();
      Serial.println("headers received");
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
  Serial.println("==========");
  Serial.println("closing connection");
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
  // Serial.println(formattedTime);

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
  Serial.print("Current date: ");
  Serial.println(currentDate);
  Serial.println("==========");

  if ((currentDate == collectionDate) && (binType == "Green")) {
    Serial.print("It's now time to take the ");
    Serial.print(binType);
    Serial.print(" bin out!!!");
    greenBin();
    Serial.print("Your next collection date is: ");
    Serial.println(nextCollection_1);
    Serial.print("Next Bin Type: ");
    Serial.print(binType_1);
  }
  else {
    Serial.print("Your next collection date is: ");
    Serial.println(collectionDate);
    Serial.print("Next Bin Type: ");
    Serial.print(binType);
  }
}


//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you ran a Full Update
    previousMillis = currentMillis;
    fullUpdate();
  }
  else {
    knightRider();
  }
}

//=======================================================================
