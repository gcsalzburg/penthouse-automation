#include <time.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <CAVE_Tasks.h>

// See credentials.h for WIFI passwords, etc
#include <credentials.h>
#include <cert.h>

// ******** SETUP FOR NODE HERE *************** //

#define NODE_NAME "living-space"

// ******************************************** //

// MQTT setup
#define MQTT_PORT   8883 // use 8883 for SSL

// Feed roote for penthouse
#define FEED_TEMP   "penthouse/temp/" NODE_NAME

// IO
#define STATUS_LED LED_BUILTIN
#define TEMP_PIN A0

// Root certificate used by MQTT server, defined in inclue/cert.h
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
WiFiClientSecure client;
#pragma GCC diagnostic pop

// MQTT client class
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Feeds
Adafruit_MQTT_Publish feed_temp = Adafruit_MQTT_Publish(&mqtt, FEED_TEMP, 0, 1);

// Function prototypes
void MQTT_check_connect(void);

// Task function prototypes
void task_send_temp();
void task_fetch_packets();
void task_check_connection();

// Table of tasks
CAVE::Task loop_tasks[] = {
   {task_send_temp, 5000},
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
   WiFi.mode(WIFI_STA); // Station mode
   WiFi.begin(WLAN_SSID, WLAN_PASS);
   while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
   }
   Serial.println();
   Serial.println("WiFi connected");
   Serial.println("IP address: "); Serial.println(WiFi.localIP());

   // Synchronize time using SNTP. This is necessary to verify that the TLS certificates offered by the server are currently valid.
   Serial.print("Setting time using SNTP");
   configTime(1, 0, "pool.ntp.org", "time.nist.gov");
   time_t now = time(nullptr);
   while (now < 8 * 3600 * 2){
      delay(500);
      Serial.print(".");
      now = time(nullptr);
   }
   Serial.println("");
   struct tm timeinfo;
   gmtime_r(&now, &timeinfo);
   Serial.print("Current time: ");
   Serial.print(asctime(&timeinfo));

   // Load root certificate in DER format into WiFiClientSecure object
   bool res = client.setCACert_P(caCert, caCertLen);
   if(!res){
      Serial.println("Failed to load root CA certificate!");
      while (true) {
         yield();
      }
   }
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

   // Verify validity of server's certificate
   if (client.verifyCertChain(MQTT_SERVER)) {
      Serial.println("Server certificate verified");
      return;
   } else {
      Serial.println("ERROR: certificate verification failed!");
      while(1);
   }
}

void task_send_temp(){
   if (!mqtt.connected()) {
      // If not connected to MQTT then don't bother getting value
      return;
   }
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