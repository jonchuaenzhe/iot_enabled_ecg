short int ecg_array[4][2800];
char buf[60000];
int count = 0;

#include "esp32-hal-adc.h" // needed for adc pin reset
#include "soc/sens_reg.h" // needed for adc pin reset

#include <WiFi.h>
#include <HTTPClient.h>

#include <TinyPICO.h>
TinyPICO tp = TinyPICO();

RTC_DATA_ATTR uint64_t reg_b; // stores the rest state before Wifi is initialised

const char* ssid = "Jon Chua";
const char* password = "12345678";

//const char* ssid = "WSiot";
//const char* password = "wirelessecg";

float voltage;

void setup() {

  tp.DotStar_SetPixelColor(0, 0, 255);

  pinMode(15, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);

  digitalWrite(21, LOW);
  digitalWrite(22, LOW);
  
  delay(1000);
  Serial.begin(115200);
  Serial.println("Wakey");

  reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);

  voltage = tp.GetBatteryVoltage();

  connect_to_wifi();

}


void loop() {
  
  read_store_ECG();

  send_data(0);

  delay(3000);

  send_data(1);

}


// reset the ADC2 pins so that it can read analog data from the ECG output
void reset_register() {

  // reset the ADC2 pins whenever a new ECG reading has to be collected
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  
}


void read_store_ECG(){
  for (int i = 0; i < 4; i++) {
    
    if (i%4 == 0) {
      digitalWrite(21, LOW);
      digitalWrite(22, LOW);
    }

    if (i%4 == 1) {
      digitalWrite(21, LOW);
      digitalWrite(22, HIGH);
    }

    if (i%4 == 2) {
      digitalWrite(21, HIGH);
      digitalWrite(22, LOW);
    }

    if (i%4 == 3) {
      digitalWrite(21, HIGH);
      digitalWrite(22, HIGH);
    }

    delay(3000);
    
    for (int j = 0; j < 700; j++) {
      reset_register();
      ecg_array[0][i*700 + j] = analogRead(15);
      reset_register();
      ecg_array[1][i*700 + j] = analogRead(27);
      reset_register();
      ecg_array[2][i*700 + j] = analogRead(26);
      reset_register();
      ecg_array[3][i*700 + j] = analogRead(25);
  
      delay(5);
    }

    delay(3000);
  }
  
}


void write_ecg_json() {
  count = -1;
  add_string("{\"ecg_data\":[");
  
  for (int i = 0; i < 4; i++) {
    add_string("[");
    
    for (int j = 0; j < 2800; j++) {
      add_string(String(ecg_array[i][j], DEC));
      
      if (j != 2799) {
        add_string(",");
      }
    }

    add_string("]");
    
    if (i != 3) {
      add_string(",");
    }
  }

  add_string("]}");
  end_buf();
}


void write_battery_json() {
  voltage = tp.GetBatteryVoltage();

  count = -1;
  add_string("{\"status\":\"alive\",\"voltage\":" + String(voltage, DEC) + "}");
  end_buf();
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


void send_data(int x) {
  HTTPClient http;

  if (x == 0) {
    write_ecg_json();
    http.begin("http://128.199.106.255:5000/api/data/ecg/1");
  } else {
    write_battery_json();
    http.begin("http://128.199.106.255:5000/api/battery/1");
  }
  
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(buf);

  Serial.println(buf);
  Serial.println(httpResponseCode);
}


void add_string(String txt) {
  int len = txt.length();
  
  for (int k = 0; k < len; k++) {
    count++;
    buf[count] = txt[k];
  }
}

void end_buf() {
  count++;
  buf[count] = '\0';
}
