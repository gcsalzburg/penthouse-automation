# Penthouse Automation #

IoT Automation for blinds

Uses `PlatformIO` VSCode integration.
Code written for `Sparkfun Pro Micro 5V/16Hz`

## Keep MQTT alive ##

```
// ping the server to keep the mqtt connection alive
// NOT required if you are publishing once every KEEPALIVE seconds
/*
if(! mqtt.ping()) {
    mqtt.disconnect();
}
*/
```