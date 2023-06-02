#include <WiFi.h>
#include <PubSubClient.h>
#include <sstream>  // std::stringstream
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "DHT.h"

DHT_Unified dht(14, DHT11);

const char* ssid = "duboo iPhone";
const char* password = "12345678";
const char* mqttServer = "34.127.102.103";
const int mqttPort = 1883;
const char* mqttUser = "yourMQTTuser";
const char* mqttPassword = "yourMQTTpassword";
const char* topic_pub = "top1";
const char* topic_sub = "top2";

#define UPDATE_INTERVAL 5000L
unsigned long prevUpdateTime = 0L;

WiFiClient espClient;
PubSubClient mqttClient(espClient);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic_sub);

  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");
}


void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");

    if (mqttClient.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, "ESP32 logged in");
  prevUpdateTime = millis();
}

void loop() {
  mqttClient.loop();
  sensors_event_t event;

  unsigned long currentTime = millis();
  if (currentTime > prevUpdateTime + UPDATE_INTERVAL) {
    dht.temperature().getEvent(&event);
    float h = event.temperature;
    dht.humidity().getEvent(&event);
    float t = event.relative_humidity;
    std::stringstream st;
    st << h;
    st << " ";
    st << t;
    mqttClient.publish(topic_pub, st.str().c_str());
    prevUpdateTime = currentTime;
    // Serial.print(F("Humidity: "));
    // Serial.print(t);
    // Serial.print(F("%  Temperature: "));
    // Serial.print(h);
    // Serial.print(F("°C "));
  }
}
