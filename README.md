# Penthouse Automation #

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