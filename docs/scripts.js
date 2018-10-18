// Create a client instance
client = new Paho.MQTT.Client("iot.eclipse.org", 443,"/ws", "gcsalzburg"+new Date().getTime());

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// connect the client
document.getElementById("status").innerHTML = "Connecting...";
client.connect({
    onSuccess:onConnect,
    useSSL:true
});

//buttons
document.getElementById("south_down").onclick  = function(e){
    sendBlind(1);
    return false;
};
document.getElementById("south_up").onclick  = function(e){
    sendBlind(2);
    return false;
};
document.getElementById("north_down").onclick  = function(e){
    sendBlind(3);
    return false;
};
document.getElementById("north_up").onclick  = function(e){
    sendBlind(4);
    return false;
};
document.getElementById("all_down").onclick  = function(e){
    sendBlind(5);
    return false;
};
document.getElementById("all_up").onclick  = function(e){
    sendBlind(6);
    return false;
};
document.getElementById("stop").onclick  = function(e){
    sendBlind(0);
    return false;
};

function sendBlind(num){
    var message = new Paho.MQTT.Message(num+"");
    message.destinationName = "gcsalzburg/blinds";
    message.qos = 0;
    console.log(message);
    
    client.send(message);
}

// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  document.getElementById("status").innerHTML = "Subscribing...";
  client.subscribe("gcsalzburg/temp");
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
    document.getElementById("status").innerHTML = "Connection lost";
    if (responseObject.errorCode !== 0) {
        console.log("onConnectionLost:"+responseObject.errorMessage);
    }
}

// called when a message arrives
function onMessageArrived(message) {
    document.querySelector('#temp').innerHTML = message.payloadString+"°";
    document.getElementById("status").innerHTML = "Temp received!";
}