/*
 *  Simple MQTT subscriber example with reconnect
 */
#include <PubSubClient.h>
#include "WiFi.h"

// Change the following to match your network settings
const char* WIFI_SSID = "change-me";
const char* WIFI_PASSWORD = "change-me";

const char* MQTT_BROKER_SERVER = "192.168.0.10";
const int MQTT_BROKER_PORT = 1883;

const char* MQTT_MESSAGE_STATUS_TOPIC = "hm/test/esp32";
const char* MQTT_MESSAGE_SUB_TOPIC = "hm/#";

const int MQTT_MSG_MAX_SIZE = 255;
const int SERIAL_MSG_BUFFER_SIZE = MQTT_MSG_MAX_SIZE + 32;

char mqtt_msg[MQTT_MSG_MAX_SIZE];
char serial_msg[SERIAL_MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

//  for (int i = 0; i < length && i < MQTT_MSG_MAX_SIZE; i++) {
//    Serial.print((char)payload[i]);
//  }

  int actual_len = length < MQTT_MSG_MAX_SIZE ? length : MQTT_MSG_MAX_SIZE;
  memcpy(mqtt_msg,payload,actual_len); 
  mqtt_msg[actual_len] = 0;
  
  snprintf (serial_msg, SERIAL_MSG_BUFFER_SIZE, "'%s' len=%d/%d time=%ld", mqtt_msg, actual_len, length, millis());
  Serial.println(serial_msg);
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ArduinoSubClient-";
    clientId += String(random(0xffff), HEX);
    String welcomeMessage = "hello world from " + clientId;
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(MQTT_MESSAGE_STATUS_TOPIC, welcomeMessage.c_str());
      client.subscribe(MQTT_MESSAGE_SUB_TOPIC);
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
  client.setCallback(mqtt_callback);
}

void loop()
{
  // MQTT loop and reconnect if connection is lost  
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();
}
