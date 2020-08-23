#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

int capacity = JSON_ARRAY_SIZE(4) + 4*JSON_ARRAY_SIZE(2000) + JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);

const char* ssid = "Jon Chua";
const char* password = "12345678";

void setup() {
  
  Serial.begin(115200);

  read_store_ECG();
  serializeJsonPretty(doc, Serial);

  connect_to_wifi();

  send_data();

}


void loop() {
  // put your main code here, to run repeatedly:

}


void read_store_ECG(){
  doc["sample_no"] = 3;
  JsonArray ecg_data = doc.createNestedArray("ecg_data");

  JsonArray channel_1 = ecg_data.createNestedArray();
  JsonArray channel_2 = ecg_data.createNestedArray();
  JsonArray channel_3 = ecg_data.createNestedArray();
  JsonArray channel_4 = ecg_data.createNestedArray();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 500; j++) {
      channel_1.add(analogRead(15)/4095);
      channel_2.add(analogRead(27)/4095);
      channel_3.add(analogRead(26)/4095);
      channel_4.add(analogRead(25)/4095);
  
      delay(5);
    }
  }
  
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


void send_data() {
  String json;
  serializeJson(doc, json);

  HTTPClient http;
 
  http.begin("https://eg3301r-ecg.herokuapp.com/api/data");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  Serial.println(httpResponseCode);
}
