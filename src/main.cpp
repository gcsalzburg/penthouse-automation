#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi setup
#define WLAN_SSID   "thepenthouse"
#define WLAN_PASS   "faradaythecatL0("

// MQTT setup
#define MQTT_SERVER "broker.hivemq.com"
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

void setup() {
  Serial.begin(115200);
  delay(150);

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

  // Setup MQTT subscriptions.
  mqtt.subscribe(&feed_blinds);
  mqtt.subscribe(&feed_led);
}

uint32_t x=0;

void loop() {
  // Check if connected, if not: connect!
  MQTT_check_connect();

  //
  // Do stuff here
  //

  // Read in subscribed topics
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &feed_blinds) {
      Serial.print(F("Got: "));
      Serial.println((char *)feed_blinds.lastread);
    }else if(subscription == &feed_led) {
      Serial.print(F("Got: "));
      Serial.println((char *)feed_led.lastread);
      if ((char *)feed_led.lastread == "1"){
        digitalWrite(STATUS_LED,LOW);
      }else if ((char *)feed_led.lastread == "0"){
        digitalWrite(STATUS_LED,HIGH);
      }
    }
  }

  // Publish to subscribed topics
  Serial.print(F("\nSending temperature: "));
  Serial.print(x);
  if (! feed_temp.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
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