//Inspired by code at https://diyi0t.com/soil-moisture-sensor-tutorial-for-arduino-and-esp8266/ as well as by ideas from many other projects.
//Compatibility reference https://www.domoticz.com/wiki/Domoticz_API/JSON_URL%27s#Soil_Moisture
//At this point, mostly written by https://codingfleet.com/code-assistant
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

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

// MQTT topics
const char* domoticz_topic = "domoticz/in"; // Domoticz MQTT topic

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// Store the last published values for each sensor
int lastPublishedValues[6] = {0, 0, 0, 0, 0, 0};

// Define the idx for each sensor
int sensorIdx[] = {506, 507, 508, 509, 510, 511}; // Replace with actual Domoticz idx for each sensor

// Connect to WiFi and MQTT
void connectToNetwork() {
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

  // Set the MQTT client ID, username, and password
  mqttClient.setId(clientID);
  mqttClient.setUsernamePassword(mqtt_username, mqtt_password);

  // Connect to the MQTT server
  while (!mqttClient.connect(mqtt_server, 1883)) {
    Serial.print(".");
    delay(5000);
  }
  Serial.println("Connected to MQTT");
}

// Setup function
void setup() {
  Serial.begin(9600);
  connectToNetwork();
  Serial.setTimeout(3000);
  // No need for MQTT discovery message for Domoticz
}

// Loop function
void loop() {
  // Keep the MQTT client connected
  if (!mqttClient.connected()) {
    connectToNetwork();
  }
  mqttClient.poll();

  // Read sensor values, average them, and publish to MQTT if there's a change
  for (int i = 0; i < 6; i++) {
    int sum = 0;
    for (int j = 0; j < 10; j++) {
      sum += analogRead(SensorPin[i]);  // Sum the readings
    }
    int average = sum / 10;  // Calculate the average

    // Divide by 15 and clamp the value between 0 and 200
    int processedValue = average / 15;
    processedValue = max(0, min(processedValue, 200));

    // Check if the processed value has changed and publish it
    if (processedValue != lastPublishedValues[i]) {
      lastPublishedValues[i] = processedValue; // Update the stored value for this sensor

      // Print sensor data to serial
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" Moisture: ");
      Serial.println(processedValue);

      // Prepare the JSON payload for Domoticz
      DynamicJsonDocument doc(1024);
      doc["idx"] = sensorIdx[i];
      doc["nvalue"] = processedValue; // nvalue is not used for custom sensors, but it must be present
      doc["svalue"] = String(processedValue); // svalue is the string representation of the value

      // Convert the JSON object to a string
      String payload;
      serializeJson(doc, payload);

      // Publish data to MQTT for Domoticz
      mqttClient.beginMessage(domoticz_topic);
      mqttClient.print(payload);
      mqttClient.endMessage();
    }
  }

  delay(5000);  // Pause for 5 seconds (5,000 milliseconds)
  // Sleep for 1 hour (60 minutes * 60 seconds * 1000 milliseconds)
  ESP.deepSleep(60 * 60 * 1e6);
}
