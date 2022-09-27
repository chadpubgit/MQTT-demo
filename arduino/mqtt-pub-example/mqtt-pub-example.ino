/*
 *  Simple MQTT publisher example with reconnect
 */
#include <PubSubClient.h>
#include "WiFi.h"

// Change the following to match your network settings
const char* WIFI_SSID = "change-me";
const char* WIFI_PASSWORD = "change-me";

const char* MQTT_BROKER_SERVER = "192.168.0.10";
const int MQTT_BROKER_PORT = 1883;

const char* MQTT_MESSAGE_TOPIC = "hm/test/esp32";
const char* MQTT_MESSAGE_CONTENT = "ESP32 Test message";

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ArduinoClient-";
    clientId += String(random(0xffff), HEX);
    String welcomeMessage = "hello world from " + clientId;
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(MQTT_MESSAGE_TOPIC, welcomeMessage.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(5);
  randomSeed(micros());
  
  Serial.print("\nConnecting to ");
  Serial.println(WIFI_SSID);
 
  WiFi.begin((char*) WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected - IP address:");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();

  // MQTT setup
  client.setServer(MQTT_BROKER_SERVER, MQTT_BROKER_PORT);
}

void loop()
{
  // MQTT loop and reconnect if connection is lost  
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();
  
  // Publish a MQTT test message
  client.publish(MQTT_MESSAGE_TOPIC, MQTT_MESSAGE_CONTENT);

  // Wait a bit before scanning again
  delay(5000);
}
