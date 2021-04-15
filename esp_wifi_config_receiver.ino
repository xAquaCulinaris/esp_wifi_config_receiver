#include <ESP8266WiFi.h>
#include <espnow.h>


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
  
}
