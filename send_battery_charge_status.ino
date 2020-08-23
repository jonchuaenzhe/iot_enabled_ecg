#include <TinyPICO.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

int capacity = JSON_OBJECT_SIZE(1);
DynamicJsonDocument doc(capacity);

const char* ssid = "Jon Chua";
const char* password = "12345678";

TinyPICO tp = TinyPICO();

void setup() {
  
  Serial.begin(115200);

  connect_to_wifi();

}

void loop() {
  
  if (analogRead(34) == 0) // 0 is charging
    tp.DotStar_SetPixelColor(0, 255, 0);
  else
    tp.DotStar_SetPixelColor(255, 0, 0);

  store_charging_voltage();
  serializeJsonPretty(doc, Serial);

  send_data();

  delay(100);

}


void connect_to_wifi() {
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
}


void store_charging_voltage() {
  doc["value"] = tp.GetBatteryVoltage();
}


void send_data() {
  String json;
  serializeJson(doc, json);

  HTTPClient http;
 
  http.begin("http://128.199.106.255:5000/api/test");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  Serial.println(httpResponseCode);
}
