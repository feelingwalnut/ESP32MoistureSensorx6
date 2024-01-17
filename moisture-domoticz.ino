//Inspired by code at https://diyi0t.com/soil-moisture-sensor-tutorial-for-arduino-and-esp8266/ as well as by ideas from many other projects.
//Compatibility reference https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Soil_Moisture
#include "WiFi.h"
#include "PubSubClient.h"

// Define sensor pins
int SensorPin[] = {36, 39, 34, 35, 32, 33};

// Define WiFi and MQTT parameters
const char* ssid = "xxxxxxxx";
const char* wifi_password = "xxxxxxxxxxxx";
const char* mqtt_server = "192.168.1.xx";
const char* mqtt_username = "xxxxxxxxxxxxxx";
const char* mqtt_password = "xxxxxxxx";
const char* clientID = "client_Plant";
const char* plant_topic = "waterplants";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker

// Connect to MQTT
void connect_MQTT() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(600);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT");
  } else {
    Serial.println("MQTT connection failed");
  }
}

// Setup function
void setup() {
  Serial.begin(9600);
  connect_MQTT();
  Serial.setTimeout(3000);

  // Read sensor values and publish to MQTT with unique ID
  for (int i = 0; i < 6; i++) {
    int readings[5];
    for (int j = 0; j < 5; j++) {
      readings[j] = analogRead(SensorPin[i]);  // Take 5 readings
    }
    int sum = 0;
    for (int j = 0; j < 5; j++) {
      sum += readings[j];
    }
    int average_value = sum / 5;  // Calculate average
    average_value /= 15;  // Divide the average by 15
    average_value = max(0, min(200, average_value));  // Ensure it does not go below 0 or above 200
 
    // Print sensor data to serial
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" average value:");
    Serial.println(average_value);
    
// Publish data to MQTT with subtopic "moisture" for each plant
char topic[40];
char payload[50];
sprintf(topic, "%s/sensor/Plant%d/moisture", plant_topic, i);  // Include sensor ID and subtopic in the topic
sprintf(payload, "%d", average_value);
client.publish(topic, payload);
}
  
  delay(10000);  // Pause for 10 seconds (10,000 milliseconds)
  // Sleep for 1 hour (60 minutes * 60 seconds * 1000 milliseconds)
  ESP.deepSleep(60 * 60 * 1e6);
}

// Loop function
void loop() {
}
