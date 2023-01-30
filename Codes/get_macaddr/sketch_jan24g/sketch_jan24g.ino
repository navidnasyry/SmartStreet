
#include <ESP8266WiFi.h>


void setup(){
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){
  digitalWrite(LED_BUILTIN, LOW);   
  Serial.println(WiFi.macAddress());  
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);   
}