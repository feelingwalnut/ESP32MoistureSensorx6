//Inspired by code at https://diyi0t.com/soil-moisture-sensor-tutorial-for-arduino-and-esp8266/ as well as by ideas from many other projects.
#include "WiFi.h"
#include "PubSubClient.h"

// Define sensor pins
int SensorPin[] = {36, 39, 34, 35, 32, 33};

// Define wet and dry thresholds for each sensor (the values of my capacitive sensors pull down with moisture, some projects show the opposite)
int wetThreshold[] = {1100, 1100, 1100, 1100, 1100, 1100};  // Define wet thresholds for each sensor
int dryThreshold[] = {2600, 2600, 2600, 2600, 2600, 2600};  // Define dry thresholds for each sensor

// Define WiFi and MQTT parameters
const char* ssid = "xxxxxxxxxx";
const char* wifi_password = "xxxxxxxxxx";
const char* mqtt_server = "192.168.1.xx";
const char* mqtt_username = "xxxxxxxxxx";
const char* mqtt_password = "xxxxxxxxxx";
const char* clientID = "client_Plant";
const char* plant_topic = "plants";

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

    
   // Check if the sensor is wet, dry, or OK based on the thresholds
 String condition;
 if (average_value < wetThreshold[i]) {
   condition = "Wet";
 } else if (average_value > dryThreshold[i]) {
   condition = "Dry";
 } else {
   condition = "OK";
 }
 
    // Print sensor data to serial
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" average value:");
    Serial.print(average_value);
    Serial.print(", condition:");
    Serial.println(condition);
 // Publish data to MQTT in JSON format
 char topic[20];
 char payload[50];
 sprintf(topic, "%s/sensor%d", plant_topic, i);  // Include sensor ID in the topic
 sprintf(payload, "{\"average_value\": %d, \"condition\": \"%s\"}", average_value, condition.c_str());
 client.publish(topic, payload);
  }
  
  delay(10000);  // Pause for 10 seconds (10,000 milliseconds)
  // Sleep for 1 hour (60 minutes * 60 seconds * 1000 milliseconds)
  ESP.deepSleep(60 * 60 * 1e6);
}

// Loop function
void loop() {
}
