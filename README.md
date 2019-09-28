# Penthouse Temperature Node #

Passive monitoring node for Penthouse automation.

Uses `PlatformIO` VSCode integration.
Code written for `ESP8266 NodeMCU ESP-12E`

## How to install

1. Clone this repo

2. Fill in Wifi and MQTT server details in `credentials.h-TEMPLATE`.

   In order to connect over TLS/SSL, the SHA1 fingerprint of the security certificate for the MQTT broker server must be set. This can be found on the server by running the command (replace domain name with your server domain name):

   `> echo | openssl s_client -servername mqtt.example.com -connect mqtt.example.com:8883 2>/dev/null | openssl x509 -noout -fingerprint`

   To test: if SHA1 fingerprint changes when certificate is renewed by Let's Encrypt.

3. In order to publish messages to the MQTT broker with a retain flag, this script uses a [forked version of the Adafruit MQTT library](https://github.com/gcsalzburg/Adafruit_MQTT_Library_retain). Clone and save this to folder in a different root folder to this project, and then set the `lib_extra_dirs` parameter to this root folder in `platformio.ini`.

   For example:
   
   Folder structure
   ```
   |--platformio-projects
      |--penthouse-temp-node
      |--another_cool_project
   |--platformio-libraries
      |-- Adafruit_MQTT_Library_retain
   ```
   
   Then in `platformio.ini` add
   
   ```
   lib_extra_dirs = D:\Development\PlatformIO-libraries\
   ```

If you don't have an MQTT broker set up on a server yet, use one of these for testing purposes: https://mntolia.com/10-free-public-private-mqtt-brokers-for-testing-prototyping/.


## Development notes for future ##

```
// ping the server to keep the mqtt connection alive
// NOT required if you are publishing once every KEEPALIVE seconds
/*
if(! mqtt.ping()) {
    mqtt.disconnect();
}
*/
```