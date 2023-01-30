#include <ESP8266WiFi.h>
#include <espnow.h>

// #define DEBUG_MODE 1      // comment on production

const int ledPin = 2;

uint8_t LEDStatus;

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&LEDStatus, incomingData, sizeof(LEDStatus));
#ifdef DEBUG_MODE
  Serial.print("led_status: ");
  Serial.println(LEDStatus);
  Serial.println();
#endif
  if (LEDStatus == 2) {
    analogWrite(ledPin, 255);  
  } else if (LEDStatus == 1) {
    analogWrite(ledPin, 15);  
  } else{
    analogWrite(ledPin, 0);  
  } 
  delay(10);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}