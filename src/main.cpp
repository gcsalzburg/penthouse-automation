#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <kTasks.h>

const char* ssid = "thepenthouse";
const char* password =  "faradaythecatL0(";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

bool reconnecting = false;

WiFiClient espClient;
PubSubClient client(espClient);

// Task function prototypes
void task_mqtt_loop(struct LoopTimer* t);

// Table of tasks
LoopTimer_t customtasktbl[] = {
		{0, 300, 0, task_mqtt_loop}
};



//
//
// TODO: Test with Adafruit MQTT Library
//
//




// Function prototypes
void callback(char* topic, byte* payload, unsigned int length);

void task_mqtt_loop(struct LoopTimer* t){
    if(!reconnecting){
        client.loop();
        if (!client.connected()){
            reconnecting = true;
            Serial.println("Reconnecting...");
            if (client.connect("ESP8266Client")) {
                // Once connected, publish an announcement...
                client.publish("esp/test", "Hello from ESP8266");
                Serial.print("Subscribe:");
                Serial.println(client.subscribe("esp/test"));
            }
            Serial.print("Connected:");
            Serial.println(client.connected());
            reconnecting = false;
        }
    }
}

 
void setup() {
 
  Serial.begin(9600);
  delay(500);

  
    tasks_register(customtasktbl);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.publish("esp/test", "Hello from ESP8266");
  Serial.print("Subscribe:");
  Serial.println(client.subscribe("esp/test"));
 
}

void callback(char* topic, byte* payload, uint32_t length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (uint32_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

void loop() {
	tasks_update();
}