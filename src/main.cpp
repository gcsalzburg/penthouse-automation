#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "credentials.h"

// See credentials.h for WIFI passwords, etc

// MQTT setup
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT   1883 // use 8883 for SSL

// Feed names
#define FEED_TEMP   "gcsalzburg/temp"
#define FEED_BLINDS "gcsalzburg/blinds"
#define FEED_LED    "gcsalzburg/led"

// IO
#define STATUS_LED LED_BUILTIN

// ESP8266 WiFiClient class
WiFiClient client;

// MQTT client class
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT);

// Feeds
Adafruit_MQTT_Publish feed_temp = Adafruit_MQTT_Publish(&mqtt, FEED_TEMP);
Adafruit_MQTT_Subscribe feed_blinds = Adafruit_MQTT_Subscribe(&mqtt, FEED_BLINDS);
Adafruit_MQTT_Subscribe feed_led = Adafruit_MQTT_Subscribe(&mqtt, FEED_LED);

// Function prototypes
void MQTT_check_connect(void);
void feed_blinds_callback(char *data, uint16_t len);
void feed_led_callback(double val);

void setup() {
  Serial.begin(115200);
  delay(150);

  pinMode(STATUS_LED, OUTPUT); 

  Serial.println(); Serial.println();
  Serial.println(F("Penthouse Automation :-)"));
  Serial.println(); Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  // Connect to WiFi access point.
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Set callbacks for subscriptions
  feed_blinds.setCallback(feed_blinds_callback);
  feed_led.setCallback(feed_led_callback);

  // Begin MQTT subscriptions.
  mqtt.subscribe(&feed_blinds);
  mqtt.subscribe(&feed_led);
}

uint32_t x=0;

int outputpin= A0;

void loop() {
  // Check if connected, if not: connect!
  MQTT_check_connect();

  //
  // Do stuff here
  //

  // Fetch subscriptions
  mqtt.processPackets(2000);

  // Publish to subscribed topics
  int analogValue = analogRead(outputpin);
  float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
  float celsius = millivolts/10;
  Serial.print(celsius);
  Serial.print("°");
  if(!feed_temp.publish(celsius)) {
    Serial.println(F(" | Err"));
  } else {
    Serial.println(F(" | Sent!"));
  }

}

// Connect / reconnect to the MQTT server.
void MQTT_check_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Connecting to MQTT... "));

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 2 seconds..."));
       mqtt.disconnect();
       delay(2000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         Serial.println(F("Giving up. Game over!"));
         while (1);
       }
  }
  Serial.println(F("MQTT Connected!"));
}

// Callbacks for subscriptions
void feed_led_callback(double val) {
  Serial.print("LED value received: ");
  Serial.print(val);
  if(val >= 1){
    Serial.println(" (ON)");
    digitalWrite(STATUS_LED,LOW);
  }else if(val < 1){
    Serial.println(" (OFF)");
    digitalWrite(STATUS_LED,HIGH);
  }
}

void feed_blinds_callback(char *data, uint16_t len) {
  Serial.print("Blinds command received: ");
  Serial.println(data);
}