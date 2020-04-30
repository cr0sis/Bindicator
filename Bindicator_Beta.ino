/*
   Bindicator by cr0sis 2020 all rights reserved.
   Got the copyright police on it an' all.
   Be cool to each other.*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/* Set these to your desired credentials. */
const char *ssid = "******";  //ENTER YOUR WIFI SETTINGS
const char *password = "*******";


//Web/Server address to read from
const char *host = "ilancasterapi.lancaster.ac.uk";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate
const char fingerprint[] PROGMEM = "F2 F4 7C 52 1F 55 DD 06 6B E6 A0 6D E4 D4 6C 17 83 5F BC DF";


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

//Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

constexpr size_t capacity = 4 * JSON_ARRAY_SIZE(1) + 5 * JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 9 * JSON_OBJECT_SIZE(2) + 560;
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(9600);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
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
  digitalWrite(LED_BUILTIN, HIGH);
}

void urgentTwinkling() {              // TIME TO TAKE OUT THE TRASH!
  for (int i = 0; i < 5000; i++) {
    digitalWrite(LED_BUILTIN, LOW);   // LOW is the voltage level; the LED is on     Flash the LED if it's Bin day!
    delay(30);
    digitalWrite(LED_BUILTIN, HIGH);  // HIGH voltage; the LED is off
    delay(70);
  }
}

void flash() {          // Flash once every update (10 seconds) to show power/updating.
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(9959);
  }
}
String getJson() {
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);

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
  Link = "//production/api/v1/CityAndRegion/Bins/Collection/house20%address"; // No help finding this for yourself sorry.
                                                                            //Your local council would be the first place I'd email
  Serial.println("requesting URL: ");
  Serial.println(host + Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  Serial.println("request sent");

  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while (httpsClient.available()) {
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

  String json = line;

  return json;

  //  const char* root_0_BinTypes_1 = doc[0]["BinTypes"][1]; // "Recycle"

  //  const char* root_1_CollectionDate = doc[1]["CollectionDate"]; // "2020-05-09T00:00:00Z"

  //  const char* root_1_BinTypes_0 = doc[1]["BinTypes"][0]; // "Refuse" MIGHT USE THIS CODE LATER TO SHOW NEXT WEEKS COLLECTION SOMEHOW.
}


//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  DynamicJsonDocument doc(capacity);
  String json = getJson();
  String  collectionDate = "";

  if(json == "") {
     // THINK ABOUT notifying user of error in fetching data, e.g. by turning on a RED LED somewhere in future.
     return;
  }

  deserializeJson(doc, json);
  String root_0_CollectionDate_0 = doc[0]["CollectionDate"]; // "2020-05-01T00:00:00Z"
  String root_0_BinTypes_0 = doc[0]["BinTypes"][0];
  Serial.print("Next collection: ");
  collectionDate = root_0_CollectionDate_0.substring(0, root_0_CollectionDate_0.length() - 10);
  Serial.println(collectionDate);
  Serial.print("Bin Type: ");
  Serial.println(root_0_BinTypes_0);

  timeClient.update();
     // UNCOMMENT strings here - you really do not need all this information returned.
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);

  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);

  struct tm *ptm = gmtime ((time_t *)&epochTime);

  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Weekday: ");
  Serial.println(weekDay);

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon + 1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth - 1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year + 1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);
  Serial.println();


  if (currentDate == collectionDate) {
    urgentTwinkling();
  }
  else {
    flash();
  }
}

//=======================================================================
