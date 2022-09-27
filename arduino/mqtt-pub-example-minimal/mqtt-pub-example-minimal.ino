/*
 *  Simple MQTT publisher minimal example (sends a single message)
 */
#include <PubSubClient.h>
#include "WiFi.h"

// Change the following to match your network settings
const char* WIFI_SSID = "change-me";
const char* WIFI_PASSWORD = "change-me";

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  // Connect to WIFI
  WiFi.begin((char*) WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Wait 0.5 seconds before retrying
  }

  // MQTT setup
  client.setServer("192.168.0.10", 1883);

  while (!client.connected()) {
    // Create a random client ID
    String clientId = "Mini-" + String(random(0xffffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      // Publish a message
      client.publish("hm/test/mini", "Hello world!");
    } else {      
      delay(500); // Wait 0.5 seconds before retrying
    }
  }
}

void loop()
{
}
