//int max_size = ESP.getFreeHeap();
//int max_block = ESP.getMaxAllocHeap();

#include "esp32-hal-adc.h" // needed for adc pin reset
#include "soc/sens_reg.h" // needed for adc pin reset

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

RTC_DATA_ATTR uint64_t reg_b; // stores the rest state before Wifi is initialised

const char* ssid = "Jon Chua";
const char* password = "12345678";

int capacity = JSON_ARRAY_SIZE(4) + 4*JSON_ARRAY_SIZE(1250) + JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);

void setup() {
  
  delay(3000);
  Serial.begin(115200);
  Serial.println("Wakey");

  reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

  connect_to_wifi();

}


void loop() {
  
  read_store_ECG();

  send_data();

  delay(15000);

}


// reset the ADC2 pins so that it can read analog data from the ECG output
void reset_register() {

  // reset the ADC2 pins whenever a new ECG reading has to be collected
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  
}


void read_store_ECG(){
  doc.clear();
  doc["sample_no"] = 2;
  serializeJsonPretty(doc, Serial);
  JsonArray ecg_data = doc.createNestedArray("ecg_data");

  JsonArray channel_1 = ecg_data.createNestedArray();
  JsonArray channel_2 = ecg_data.createNestedArray();
  JsonArray channel_3 = ecg_data.createNestedArray();
  JsonArray channel_4 = ecg_data.createNestedArray();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 400; j++) {
      reset_register();
      channel_1.add(analogRead(15));
      reset_register();
      channel_2.add(analogRead(15));
      reset_register();
      channel_3.add(analogRead(15));
      reset_register();
      channel_4.add(analogRead(15));
  
      delay(8);
    }
  }

  serializeJsonPretty(doc, Serial);
  
}


void connect_to_wifi() {
  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
}


void send_data() {
  String json;
  serializeJson(doc, json);

  HTTPClient http;
 
  http.begin("http://128.199.106.255:5000/api/data");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  Serial.println(httpResponseCode);
}
