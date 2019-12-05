# Penthouse Temperature Node #

Passive monitoring node for Penthouse automation.

Uses `PlatformIO` VSCode integration.
Code written for `ESP8266 NodeMCU ESP-12E`

## How to install

1. Clone this repo

2. Fill in Wifi and MQTT server details in `credentials.h-TEMPLATE`.

3. Replace the data in `cert.h` with your own certificate.

   Follow these instructions to download the **root** certificate, saving it as `ca.cert` for your MQTT server domain name: https://docs.bvstools.com/home/ssl-documentation/exporting-certificate-authorities-cas-from-a-website

   On Linux run `xxd -i ca.cert ca_cert.h` to convert to the correct format. Then copy the data into `cert.h` (check that the variable names are correct).   

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

In order to publish messages to the MQTT broker with a retain flag, this script uses a [forked version of the Adafruit MQTT library](https://github.com/gcsalzburg/Adafruit_MQTT_Library_retain). You will need access to this repo to compile.

### Old fingerprinting method...

In order to connect over TLS/SSL, the SHA1 fingerprint of the security certificate for the MQTT broker server must be set. This can be found on the server by running the command (replace domain name with your server domain name):

`> echo | openssl s_client -servername mqtt.example.com -connect mqtt.example.com:8883 2>/dev/null | openssl x509 -noout -fingerprint`

~~To test: if SHA1 fingerprint changes when certificate is renewed by Let's Encrypt.~~
Turns out, obviously, they do! So we changed the method.

Useful links on this:

+ https://github.com/esp8266/Arduino/issues/1851
+ https://docs.bvstools.com/home/ssl-documentation/exporting-certificate-authorities-cas-from-a-website
+ https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/HTTPSRequestCACertAxTLS/HTTPSRequestCACertAxTLS.ino

Future links - for setting up to authenticate the nodes only:

+ Best link: https://nofurtherquestions.wordpress.com/2016/03/14/making-an-esp8266-web-accessible/
+ https://blog.thewalr.us/2019/03/27/using-esp8266-as-an-iot-endpoint-with-encrypted-mqtt-transport/