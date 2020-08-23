#include "esp32-hal-adc.h" // needed for adc pin reset
#include "soc/sens_reg.h" // needed for adc pin reset

#include <WiFi.h>

#include <HTTPClient.h>

RTC_DATA_ATTR int first_boot = 1; // indicates if it is the first startup
RTC_DATA_ATTR uint64_t reg_b; // stores the rest state before Wifi is initialised

const char* ssid = "Jon Chua";
const char* password = "12345678";

#define uS_TO_S_FACTOR 1000000 
#define TIME_TO_SLEEP  10


void setup() {
  
  delay(3000);
  Serial.begin(115200);
  Serial.println("Wakey");
  
//  reset_register();

  int i;
  for (i = 0; i < 20; ++i) {
    delay(200);
    read_ECG();
  }

  delay(3000);
  connect_to_wifi();

  WiFi.disconnect();

  delay(3000);
  connect_to_wifi();

  delay(3000);
//  deep_sleep();

}


void loop() {
  // put your main code here, to run repeatedly:

}


// reset the ADC2 pins so that it can read analog data from the ECG output
void reset_register() {

  // store the initial state of the ADC2 pins in reg_b only if it is the first time starting
  // this is because the Wifi has not been initialised yet
  if (first_boot) {
    Serial.println("First Boot");
    reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
    first_boot = 0;
  }

  // reset the ADC2 pins whenever a new ECG reading has to be collected
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  
}


// switch the multiplexers and read the ECG data into the 4 channels
void read_ECG() {

  Serial.println(analogRead(15));
  
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


// deep sleep
void deep_sleep() {
  WiFi.disconnect();
  Serial.println("Disconnected");

  Serial.println("Entering Deep Sleep");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
