#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <CAVE_Tasks.h>

// See credentials.h for WIFI passwords, etc
#include <credentials.h>

// MQTT setup
#define MQTT_PORT   8883 // use 8883 for SSL

// Feed names
#define FEED_TEMP   "penthouse/temp"

// IO
#define STATUS_LED LED_BUILTIN
#define TEMP_PIN A0

// ESP8266 WiFiClient class
WiFiClientSecure client;

// MQTT client class
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Feeds
Adafruit_MQTT_Publish   feed_temp = Adafruit_MQTT_Publish(&mqtt, FEED_TEMP);

// Function prototypes
void MQTT_check_connect(void);

// Task function prototypes
void task_send_temp();
void task_fetch_packets();
void task_check_connection();

// Table of tasks

CAVE::Task loop_tasks[] = {
   {task_send_temp, 5000},
   {task_fetch_packets, 200},
   {task_check_connection, 3000}
};


void setup() {
  Serial.begin(115200);
  delay(150);

   CAVE::tasks_register(loop_tasks);

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
}

void loop() {
	// Call task updater
	CAVE::tasks_update();
}

// Connect / reconnect to the MQTT server.
void task_check_connection() {
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

void task_fetch_packets(){
  // Fetch subscriptions
  mqtt.processPackets(200);
}

void task_send_temp(){
  int analogValue = analogRead(TEMP_PIN);
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