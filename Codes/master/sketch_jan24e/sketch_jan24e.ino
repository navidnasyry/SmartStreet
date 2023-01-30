#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <espnow.h>

// #define DEBUG_MODE 1                // comment on production
// #define DEBUG_DELAY 1000            // ms
#define SOUND_VELOCITY 0.034        // define sound velocity in cm/uS
#define ULTRASONIC_SENSITIVITY 100  // cm
#define LDR_SENSITIVITY 512         // N/A
#define DELAY_BETWEEN_CODES 10

// global variables
const char* WifiSSID = "NN";
const char* WifiPassword = "11112222";
const char* apiAddress = "http://176.101.47.236/api/account/login/";
const int UltrasonicTriggerPin = 12;
const int UltrasonicEchoPin = 14;
const int LDRPin = A0;  // Defining LDR PIN
int LEDDelay = 5000;    // ms
const int ledPin = 2;
unsigned long PreviousTime = 0;
unsigned long WifiRestartDelay = 10000;  // 10 seconds delay
uint8_t LEDStatus = 2;

uint8_t BroadcastAddressList[][6] = { { 0x84, 0xF3, 0xEB, 0x53, 0x2C, 0x00 },
                                      { 0xDC, 0x4F, 0x22, 0x4C, 0x88, 0x28 },
                                      // { 0xDC, 0x4F, 0x22, 0x4C, 0xB2, 0xFE },
                                      { 0xB4, 0xE6, 0x2D, 0x67, 0xCB, 0x2E } };

#ifdef DEBUG_MODE
void OnDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  char macStr[18];
  Serial.print("Packet to:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}
#endif

float getUltrasonicValue() {
  // Clears the trigPin
  digitalWrite(UltrasonicTriggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(UltrasonicTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltrasonicTriggerPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(UltrasonicEchoPin, HIGH);

  // Calculate the distance
  float distanceCm = duration * SOUND_VELOCITY / 2;

#ifdef DEBUG_MODE
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  delay(DEBUG_DELAY);
#endif

  return distanceCm;
}

int getLDRValue() {
  int ldr_value = analogRead(LDRPin);  // Reading Input
#ifdef DEBUG_MODE
  Serial.print("LDR value is : ");
  Serial.println(ldr_value);
  delay(DEBUG_DELAY);
#endif
  return ldr_value;
}

void initPins() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(UltrasonicTriggerPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(UltrasonicEchoPin, INPUT);      // Sets the echoPin as an Input
  digitalWrite(LED_BUILTIN, LOW);
}

void initESPNow() {
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

#ifdef DEBUG_MODE
  esp_now_register_send_cb(OnDataSent);
#endif

  for (int i = 0; i < (sizeof(BroadcastAddressList) / sizeof(BroadcastAddressList[0])); i++) {
    esp_now_add_peer(BroadcastAddressList[i], ESP_NOW_ROLE_SLAVE, 0, NULL, 0);
  }
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiSSID, WifiPassword, 0);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
}

void controlLED() {
  float distance_cm = getUltrasonicValue();
  int ldr_value = getLDRValue();

  if ((distance_cm < ULTRASONIC_SENSITIVITY) && (ldr_value > LDR_SENSITIVITY)) {
    LEDStatus = 2;
    LEDDelay = 5000;
    analogWrite(ledPin, 255);
  } else if ((distance_cm >= ULTRASONIC_SENSITIVITY) && (ldr_value > LDR_SENSITIVITY)) {
    LEDStatus = 1;
    LEDDelay = 50;
    analogWrite(ledPin, 15);
  } else {
    LEDStatus = 0;
    LEDDelay = 50;
    analogWrite(ledPin, 0);
  }
}


void checkInternetConnection() {
  unsigned long currentTime = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentTime - PreviousTime >= WifiRestartDelay)) {
#ifdef DEBUG_MODE
    Serial.print(millis());
    Serial.println("Reconnecting to WIFI network");
#endif
    WiFi.begin(WifiSSID, WifiPassword, 0);
    PreviousTime = currentTime;
  }
}

void sendDataToServer() {
  if (WiFi.status() == WL_CONNECTED && LEDStatus) {
    WiFiClient client;
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(client, apiAddress);

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "username=admin&password=Kian_@dmin";
    int httpResponseCode = http.POST(httpRequestData);

    // If you need an HTTP request with a content type: application/json, use the following:
    //http.addHeader("Content-Type", "application/json");
    //int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");

    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
#ifdef DEBUG_MODE
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
#endif
    // Free resources
    http.end();
  }
}


void setup() {
  Serial.begin(115200);
  initPins();
  initWifi();
  initESPNow();
}

void loop() {
  controlLED();
  delay(DELAY_BETWEEN_CODES);

  for (int i = 0; i < 4; i++) {
    esp_now_send(0, (uint8_t*)&LEDStatus, sizeof(LEDStatus));
    delay(DELAY_BETWEEN_CODES * 10);
  }
  // checkInternetConnection();
  // delay(DELAY_BETWEEN_CODES);

  // sendDataToServer();
  // delay(DELAY_BETWEEN_CODES * 2);

  delay(LEDDelay);
}