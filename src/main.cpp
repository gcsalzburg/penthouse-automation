#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <credentials.h>
#include "kTasks.h"

// See credentials.h for WIFI passwords, etc

// Number of blinds
#define NUM_BLINDS 2
#define NUM_RELAYS NUM_BLINDS*2

// MQTT setup
#define MQTT_PORT   8883 // use 8883 for SSL

// Feed names
#define FEED_TEMP   "penthouse/temp"
#define FEED_BLINDS "penthouse/blinds"
#define FEED_LED    "penthouse/led"

// Relay pins
#define RELAY_0 D1
#define RELAY_1 D2
#define RELAY_2 D3
#define RELAY_3 D4

// IO
#define STATUS_LED LED_BUILTIN
#define TEMP_PIN A0

// Default hold delay on up/down trigger
#define BLIND_HOLD_DELAY 1500
#define BLIND_KILL_DELAY 250

// Define structs to hold blind information
enum BlindDirection{
	NONE = 0,
	UP,
	DOWN
};

typedef struct{
  char            *blind_name;
	BlindDirection  dir;
	BlindDirection  last_dir;
	uint32_t        last_trigger;
	uint8_t	        up_pin;
	uint8_t         down_pin;
}BlindState_t;

BlindState_t north = {"north", NONE, NONE, 0, RELAY_0, RELAY_1};
BlindState_t south = {"south", NONE, NONE, 0, RELAY_2, RELAY_3};
BlindState_t blinds[] = {north, south};

// ESP8266 WiFiClient class
WiFiClientSecure client;

// MQTT client class
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Feeds
Adafruit_MQTT_Publish feed_temp = Adafruit_MQTT_Publish(&mqtt, FEED_TEMP);
Adafruit_MQTT_Subscribe feed_blinds = Adafruit_MQTT_Subscribe(&mqtt, FEED_BLINDS);
Adafruit_MQTT_Subscribe feed_led = Adafruit_MQTT_Subscribe(&mqtt, FEED_LED);

// Function prototypes
void MQTT_check_connect(void);
//void feed_blinds_callback(char *data, uint16_t len);
void feed_blinds_callback(double val);
void feed_led_callback(double val);

void blind_start(BlindState_t *blind, BlindDirection dir);
void blind_release(BlindState_t *blind);
void blind_stop(BlindState_t *blind);

void cmd_blind(uint8_t argc, char *argv[]);
void cmd_delay(uint8_t argc, char *argv[]);
void cmd_stop(uint8_t argc, char *argv[]);

// Task function prototypes
void task_send_temp(struct LoopTimer* t);
void task_check_blind_release(struct LoopTimer* t);
void task_fetch_packets(struct LoopTimer* t);
void task_check_connection(struct LoopTimer* t);

// Table of tasks
LoopTimer_t customtasktbl[] = {
		{1, 5000, 0, task_send_temp},
    {2, 200, 0, task_check_blind_release},
    {3, 200, 0, task_fetch_packets},
    {4, 3000, 0, task_check_connection}
};


void setup() {
  Serial.begin(115200);
  delay(150);

	tasks_register(customtasktbl);

  pinMode(STATUS_LED, OUTPUT); 

  // Turn off all blinds
  for(uint8_t i=0; i<NUM_BLINDS; i++){
    pinMode(blinds[i].up_pin,OUTPUT);
    pinMode(blinds[i].down_pin,OUTPUT);
    digitalWrite(blinds[i].up_pin,HIGH);
    digitalWrite(blinds[i].down_pin,HIGH);
  }

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

void loop() {
	// Call task updater
	tasks_update();
}

// Connect / reconnect to the MQTT server.
void task_check_connection(struct LoopTimer* t) {
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

void task_fetch_packets(struct LoopTimer* t){
  // Fetch subscriptions
  mqtt.processPackets(200);
}

void task_check_blind_release(struct LoopTimer* t){
  // Check if we need to cancel the blind movement
  for(uint8_t i=0; i<NUM_BLINDS; i++){
      if(blinds[i].dir > NONE){
          if(millis() > blinds[i].last_trigger+BLIND_HOLD_DELAY){
              blind_release(&blinds[i]);
          }
      }
  }
}

void task_send_temp(struct LoopTimer* t){
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

void feed_blinds_callback(double val) {
  Serial.print("Blind value received: ");
  Serial.println(val);
  if(val == 0){
    blind_stop(&blinds[0]);
    blind_stop(&blinds[1]);
  }else if(val == 1){
    blind_start(&blinds[0],DOWN);
  }else if(val == 2){
    blind_start(&blinds[0],UP);
  }else if(val == 3){
    blind_start(&blinds[1],DOWN);
  }else if(val == 4){
    blind_start(&blinds[1],UP);
  }else if(val == 5){
    blind_start(&blinds[0],DOWN);
    blind_start(&blinds[1],DOWN);
  }else if(val == 6){
    blind_start(&blinds[0],UP);
    blind_start(&blinds[1],UP);
  }
}


// Set the movement of a blind
void blind_start(BlindState_t *blind, BlindDirection dir){
    if((dir == UP) || (dir == DOWN)){   
      Serial.print("Moving ");
      Serial.print(blind->blind_name);
      Serial.print(" blind ");
      if(dir == UP){
          digitalWrite(blind->up_pin,LOW);
          digitalWrite(blind->down_pin,HIGH);
          Serial.println("up");
      }else if(dir == DOWN){
          digitalWrite(blind->up_pin,HIGH);
          digitalWrite(blind->down_pin,LOW);
          Serial.println("down");
      }
      blind->dir = dir;
      blind->last_dir = dir;
      blind->last_trigger = millis();
    }else{
      Serial.println("[No change to blinds]");
    } 
}

// Stop a blind moving
void blind_release(BlindState_t *blind){  
  Serial.print("Released ");
  Serial.print(blind->blind_name);
  Serial.println(" blind switch");

  digitalWrite(blind->up_pin,HIGH);
  digitalWrite(blind->down_pin,HIGH);
  blind->dir = NONE;
}

// Kill blind movement with short tap in other direction
void blind_stop(BlindState_t *blind){
  if((blind->last_dir == UP) || (blind->last_dir == DOWN)){   
    if(blind->last_dir == DOWN){
      digitalWrite(blind->up_pin,HIGH);
      digitalWrite(blind->down_pin,LOW);
      delay(BLIND_KILL_DELAY);
      digitalWrite(blind->down_pin,HIGH);
    }else if(blind->last_dir == UP){
      digitalWrite(blind->down_pin,HIGH);
      digitalWrite(blind->up_pin,LOW);
      delay(BLIND_KILL_DELAY);
      digitalWrite(blind->up_pin,HIGH);
    }

    Serial.print("Stopped ");
    Serial.print(blind->blind_name);
    Serial.println(" blind moving");
    
    blind->dir = NONE;
    blind->last_dir = NONE;

  }
}