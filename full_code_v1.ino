#include <TinyPICO.h>

#include "esp32-hal-adc.h" // needed for adc pin reset
#include "soc/sens_reg.h" // needed for adc pin reset

#include <ArduinoJson.h>

#include <WiFi.h>

#include <HTTPClient.h>

RTC_DATA_ATTR int first_boot = 1; // indicates if it is the first startup
RTC_DATA_ATTR uint64_t reg_b; // stores the rest state before Wifi is initialised

int channel_1[4][500]; // store ECG data for Lead I, aVR, V1, V4
int channel_2[4][500]; // store ECG data for Lead II, aVL, V2, V5
int channel_3[4][500]; // store ECG data for Lead III, aVF, V3, V6
int channel_4[2000]; // store ECG data for continuous Lead II

const int S0 = 36;
const int S1 = 38;

TinyPICO tp = TinyPICO();

const char* ssid = "Jon Chua";
const char* password = "12345678";

#define uS_TO_S_FACTOR 1000000 
#define TIME_TO_SLEEP  10


void setup() {

  // if TinyPico is charging, skip to the loop function
  if (!tp.IsChargingBattery())
    return;
  
  reset_register();

  read_ECG();

  int capacity = 11*JSON_ARRAY_SIZE(500) + JSON_ARRAY_SIZE(2000) + JSON_OBJECT_SIZE(13);
  DynamicJsonDocument doc(capacity);

  store_ECG();

  get_battery_level();

  connect_to_wifi();

  send_data();

  deep_sleep();

}


void loop() {
  int capacity = JSON_OBJECT_SIZE(1);
  DynamicJsonDocument doc(capacity);

  get_battery_level();

  if(WiFi.status() != WL_CONNECTED){
    connect_to_wifi();
  }

  send_data();

  // delay by 10 minutes (minutes * seconds * milliseconds)
  delay(10* 60 * 1000)

  if (!tp.IsChargingBattery())
    deep_sleep();
}


// reset the ADC2 pins so that it can read analog data from the ECG output
void reset_register() {

  // store the initial state of the ADC2 pins in reg_b only if it is the first time starting
  // this is because the Wifi has not been initialised yet
  if (first_boot) {
    reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
    first_boot = 0;
  }

  // reset the ADC2 pins whenever a new ECG reading has to be collected
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  
}


// switch the multiplexers and read the ECG data into the 4 channels
void read_ECG() {

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);

  for (int i = 0; i < 4; i++) { 
    for (int j = 0; j < 500; j++) {
      channel_1[i][j] = analogRead(16);
      channel_2[i][j] = analogRead(21);
      channel_3[i][j] = analogRead(17);
      channel_4[500*i + j] = analogRead(24);
  
      delay(5);
    }

    digitalWrite(S0, !digitalRead(S0));
    if (i%2 == 1)
      digitalWrite(S1, !digitalRead(S1));
  }
  
}


// add the arrays to the json document
void store_ECG() {
  doc["L1"] = channel_1[0]
  doc["aVR"] = channel_1[1]
  doc["V1"] = channel_1[2]
  doc["V4"] = channel_1[3]
  doc["L2"] = channel_4
  doc["aVL"] = channel_2[1]
  doc["V2"] = channel_2[2]
  doc["V5"] = channel_2[3]
  doc["L3"] = channel_3[0]
  doc["aVF"] = channel_3[1]
  doc["V3"] = channel_3[2]
  doc["V6"] = channel_3[3]
}


// get battery level of the tinypico
void get_battery_level() {
  doc["battery_level"] = tp.GetBatteryVoltage()
}


// connect TinyPico to Wifi
void connect_to_wifi() {
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}


// send json data to API endpoint
void send_data() {
  String json;
  serializeJson(doc, json);

  HTTPClient http;
 
  http.begin("https://eg3301r-ecg.herokuapp.com/api/data");
  http.addHeader("Content-Type", "application/json");
  http.POST(json);
}


// deep sleep
void deep_sleep() {
  WiFi.disconnect();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
