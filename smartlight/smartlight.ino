#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>


const char* ssid = "ESP32";  // SSID
const char* password = "12345678";  // PASSWORD

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

const int PIN_TO_SENSOR = 27;
const int PIN_TO_LED = 26;
const int PIN_TO_LIGHT_SENSOR = 25;
const int PIN_TO_BUTTON = 33;

int pinStateCurrent   = LOW;
int pinStatePrevious  = LOW;
int lastButtonState = HIGH;
int currentButtonState;
int motionDetectState = LOW;

const int freq = 9000;
const int ledChannel = 0;
const int resolution = 10;

//acendimento automatico, acesder e apagar a lampada, ajuste automatico de intensidade, ajustar luminosidade

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_SENSOR, INPUT);
  pinMode(PIN_TO_BUTTON, INPUT_PULLUP);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PIN_TO_LED, ledChannel);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.begin();
  Serial.println("HTTP server started");


  
  WiFiClient client;
  HTTPClient http;
  
}

void loop() {
  server.handleClient();
  currentButtonState = digitalRead(PIN_TO_BUTTON);
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    motionDetectState = !motionDetectState;
    if (motionDetectState) {
      motionDetectStateOn();
    } else {
      motionDetectStateOff();
    }
  }

  lastButtonState = currentButtonState;
  if (motionDetectState == HIGH) {
    pinStatePrevious = pinStateCurrent;
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);
    if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
      Serial.println("Motion detected!");
      calculateBright(1);
    }
    else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {
      Serial.println("Motion stopped!");
      calculateBright(0);
    }
  } else {
    calculateBright(0);
  }
  delay(100);
}
 

void calculateBright(int state) {
  if (state == 1) {
    int analogValue = analogRead(PIN_TO_LIGHT_SENSOR);
    analogValue = map(analogValue, 0, 4095, 3000, 0);
    ledcWrite(ledChannel, analogValue);
    Serial.print("Analog Value = ");
    Serial.println(analogValue);
  } else {
    ledcWrite(ledChannel, 0);
  }
}
