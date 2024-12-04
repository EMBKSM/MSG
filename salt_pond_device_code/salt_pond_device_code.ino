#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "name";     
const char* password="password"; 
const char* serverAddress = "http://255.255.0.0:8080/plant_m/connect"; 

void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  // 활성화 코드
  /*Wire.beginTransmission(0x68);
  Wire.write(107);
  Wire.write(0);
  Wire.endTransmission();*/
  // Register 27 => 각가속도 감도를 담당하는 레지스터
  Wire.beginTransmission(0x68);
  Wire.write(27);
  Wire.write(0); // => ±250deg/s(초당 최대 각가속도 감지량)
  Wire.endTransmission();
  // Register 28 => 가속도 감도를 담당하는 레지스터
  Wire.beginTransmission(0x68);
  Wire.write(28);
  Wire.write(0); // => 2g
  Wire.endTransmission();
  WiFi.begin(ssid, password);// wifi 연결 시도
  while (WiFi.status() != WL_CONNECTED) {// wifi 연결 여부 확인
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
}

void loop() {
  uint8_t i;
  int16_t acc_raw[3], gyro_raw[3];
  Wire.beginTransmission(0x68);// x,y,z 축 가속도 데이터 가져오기
  Wire.write(59);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  for(i = 0; i < 3; i++) acc_raw[i] = (Wire.read() << 8) | Wire.read();
  Wire.beginTransmission(0x68);// 각가속도 데이터 가져오기
  Wire.write(67);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 6);
  for(i = 0; i < 3; i++) gyro_raw[i] = (Wire.read() << 8) | Wire.read();
  if (WiFi.status() == WL_CONNECTED) {//서버에 데이터 보네기
      HTTPClient http;
      WiFiClient client;
      http.begin(client, serverAddress); 
      http.addHeader("Content-Type", "application/json");
      String jsonData = "{\"AccX\":"+String(acc_raw[0])+",\"AccY\":"+String(acc_raw[1])+",\"AccZ\":"+String(acc_raw[2])+",\"GyroX\":"+String(gyro_raw[0])+",\"GyroY\":"+String(gyro_raw[1])+",\"GyroZ\":"+String(gyro_raw[2])+"}"; 
      int httpResponseCode = http.POST(jsonData);// 보내는 부분
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("HTTP Response Code: " + String(httpResponseCode));
        Serial.println(response);
      } else {
        Serial.println("Error in HTTP  request");
      }

      http.end();
  }
  delayMicroseconds(100);
}