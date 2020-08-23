#include "esp32-hal-adc.h" // needed for adc pin reset
#include "soc/sens_reg.h" // needed for adc pin reset

#include <WiFi.h>

#include <HTTPClient.h>

RTC_DATA_ATTR uint64_t reg_b; // stores the rest state before Wifi is initialised

const char* ssid = "Jon Chua";
const char* password = "12345678";


void setup() {
  
  delay(3000);
  Serial.begin(115200);
  Serial.println("Wakey");

  reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

  connect_to_wifi();

}


void loop() {
  int i;
  for (i = 0; i < 1000; ++i) {
    delay(5);
    read_ECG();
  }

}


// reset the ADC2 pins so that it can read analog data from the ECG output
void reset_register() {

  // reset the ADC2 pins whenever a new ECG reading has to be collected
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  
}


// switch the multiplexers and read the ECG data into the 4 channels
void read_ECG() {

  reset_register();
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
