#include <ESP8266WiFi.h>
#include <espnow.h>

#include <Ticker.h>
#include <AsyncMqttClient.h>

//MQTT Server IP
#define MQTT_HOST IPAddress(1, 1, 1, 1)
//MQTT Server Port
#define MQTT_PORT 1

#define MQTT_USER "xxx"
#define MQTT_PASS "xxx"


//MQTT Topics
#define MQTT_PUB_TEMP "sensordata/temperature"
#define MQTT_PUB_HUM "sensordata/humidity"
#define MQTT_PUB_ID "sensordata/ID"


// Data to send
float temp;
float hum;
int id;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker wifiReconnectTimer;


unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

bool send_mqtt_data = false;



typedef struct wifi_cresedentials {
  char ssid[32];
  char pass[32];

} wifi_cresedentials;



wifi_cresedentials cresedentials;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&cresedentials, incomingData, sizeof(cresedentials));

  Serial.println("");
  Serial.print("SSID: ");
  Serial.println(cresedentials.ssid);
  Serial.print("Password: ");
  Serial.println(cresedentials.pass);
  Serial.println();
  connect_to_wifi();
}

void connect_to_wifi() {
  esp_now_deinit();
  Serial.print("Connecting to ");
  Serial.println(cresedentials.ssid);
  WiFi.hostname("sensor1");
  WiFi.begin(cresedentials.ssid, cresedentials.pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());

  connectToMqtt();
}


void connectToMqtt() {
  Serial.println();
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
  send_mqtt_data = true;
}


void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if (send_mqtt_data) {
    unsigned long currentMillis = millis();
    // Every X number of seconds (interval = 10 seconds)
    // it publishes a new MQTT message
    if (currentMillis - previousMillis >= interval) {
      // Save the last time a new reading was published
      previousMillis = currentMillis;
      temp = 22.0;
      hum = 32.2;
      id = 01;

      // Publish an MQTT message on topic esp/bme280/temperature
      uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub1);
      Serial.printf("Message: %.2f \n", temp);

      // Publish an MQTT message on topic esp/bme280/humidity
      uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_HUM, packetIdPub2);
      Serial.printf("Message: %.2f \n", hum);

      // Publish an MQTT message on topic esp/bme280/pressure
      uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_ID, 1, true, String(id).c_str());
      Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_ID, packetIdPub3);
      Serial.printf("Message: %.3f \n", id);
    }
  }

}
