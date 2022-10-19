#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "ESP32XU";  // SSID
const char* password = "12345678";  // PASSWORD

// fix ip configuration
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Web server running on port 80
WebServer server(80);

// pins
const int PIN_TO_LIGHT_SENSOR = 32;
const int PIN_TO_LED = 26;
const int PIN_TO_BUTTON = 33;
const int PIN_TO_PIR_SENSOR = 35;

// variables
bool is_automatic_bright = true;
int led_bright = 0;

int button_state = HIGH;
int last_button_state = HIGH;

bool is_motion_detection_active = false;
bool had_motion_detected = false;
unsigned long last_motion_detected = 0;
int motion_check_interval = 5000; // interval between motion checks


unsigned long now = millis();


// pwd setup
const int led_pwm_frequency = 9000;
const int led_pwm_channel = 0;
const int led_pwm_resolution = 10;


// this function is called every time that a movement is detected
void IRAM_ATTR onMotionDetected() {
  if (is_motion_detection_active) {
    Serial.println("Motion Detected");
    had_motion_detected = true;
    last_motion_detected = millis();  

    // turn on the light
    is_automatic_bright = true;
  }
}


// setup API routes
void setup_routing() {

    // app root page
    server.on("/", HTTP_GET, []() {
      server.send(200, "text/html","<!DOCTYPE html><html lang='pt-BR'><head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <title>Smart Light</title></head><body> <section class='section'> <div class='container'> <h1 class='title'> Smart Light </h1> <p class='subtitle'> Controle sua lâmpada aqui! </p></div></section> <section class='section'> <div class='container'> <form> <div class='field'> <label class='label'>Luminosidade:</label> <div class='control'> <input id='lumi' class='input' type='number' max='1024' min='0' placeholder='Ex.: 50' onchange='onChangeLuminosityValue'/> </div></div><div class='field is-grouped'> <div class='control'> <button class='button is-success' type='button' onclick='sendChangeLuminosity()'> Enviar </button> </div></div><div class='is-divider'></div><div class='field'> <div class='control'> <button class='button is-success' type='button' onclick='sendEnableLDR()'> Ativar sensor de luminosidade </button> </div></div><div class='is-divider'></div></form> </div></section></body><script>function onChangeLuminosityValue(){console.log('safado');}async function sendEnableLDR(){const url='http://192.168.1.1/led-auto-bright'; const options={method: 'POST', mode: 'no-cors', cache: 'no-cache', body: true,}; const response=await fetch(url, options); console.log(response);}async function sendChangeLuminosity(){const luminosityInput=document.getElementById('lumi'); if (!luminosityInput){return;}if (luminosityInput.value > 1024){alert('O valor de ser entre 0 e 1024'); return;}const url='http://192.168.1.1/led-bright'; const options={method: 'POST', mode: 'no-cors', cache: 'no-cache', body: luminosityInput.value,}; const response=await fetch(url, options); console.log(response);}</script><style>body,button,input,optgroup,select,textarea{font-family: BlinkMacSystemFont, -apple-system, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans', 'Droid Sans', 'Helvetica Neue', 'Helvetica', 'Arial', sans-serif;}</style></html>");
    });

    // define led bright
    server.on("/led-bright", HTTP_POST, []() {
      String value = server.arg("plain");

      Serial.print("Value from led-bright endpoint: ");
      Serial.println(value);

      is_automatic_bright = false;
      led_bright = value.toInt();

      Serial.println("Enter on led bright MANUAL mode");

      server.send(200, "text/plain", value);
    });

    // define bright auto ajust
    server.on("/led-auto-bright", HTTP_POST, []() {
      String value = server.arg("plain");

      Serial.print("Value from led-bright endpoint");
      Serial.println(value);

      if (value == "true") {
        Serial.println("Enter on led bright AUTOMATIC mode");
        is_automatic_bright = true;  
      } else {
        Serial.println("Enter on led bright MANUAL mode");
        is_automatic_bright = false;
      }

      server.send(200, "text/plain", value);
    });

    // active and inactive motion detection mode
    server.on("/motion-detection", HTTP_POST, []() {
      String value = server.arg("plain");

      Serial.print("is motion detection active");
      Serial.println(value);

      if (value == "true") {
        Serial.println("Enter on Motion detection mode");
        is_motion_detection_active = true;  
        onMotionDetected();
      } else {
        Serial.println("Out of motion detection mode");
        is_motion_detection_active = false;  
      }

      server.send(200, "text/plain", value);
    });

    // define motion detection delay
    server.on("/motion-detection-delay", HTTP_POST, []() {
      String value = server.arg("plain");

      Serial.print("Motion detection new delay is: ");
      Serial.println(value);

      motion_check_interval = value.toInt();

      server.send(200, "text/plain", value);
    });

}


void setup() {
  Serial.begin(115200);

  // setup pins
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_BUTTON, INPUT_PULLUP);
  pinMode(PIN_TO_PIR_SENSOR, INPUT_PULLUP);

  // setup pwm
  ledcSetup(led_pwm_channel, led_pwm_frequency, led_pwm_resolution);
  ledcAttachPin(PIN_TO_LED, led_pwm_channel);

  // setup interrup function
   attachInterrupt(digitalPinToInterrupt(PIN_TO_PIR_SENSOR), onMotionDetected, FALLING);


  // wifi setup
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");

  // setup ap
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.enableCORS(true);
  setup_routing();

  // start server
  server.begin();
}

void loop() {
  now = millis();
  
  server.handleClient();

  // --- start led bright
  if (is_automatic_bright) {
    autoDefineLEDBright();  
  } else {
    ledcWrite(led_pwm_channel, led_bright);
  }
  // --- endled bright

  // --- start button
  int current_button_state = digitalRead(PIN_TO_BUTTON);

  // if button changed state
  if (current_button_state == HIGH && last_button_state == LOW) {
    // change state by button
    if (button_state == HIGH) {
      button_state = LOW;
      Serial.println("Button changed to LOW");
      is_automatic_bright = false;
      led_bright = 0;
    } else {
      button_state = HIGH;
      Serial.println("Button changed to HIGH");
      is_automatic_bright = true;
    } 
  }

  last_button_state = current_button_state;
  // --- end button

  // --- start PIT sensor
  
  // clear has movement after delay 
  if (is_motion_detection_active && had_motion_detected && (now - last_motion_detected > motion_check_interval)) {
    Serial.println("Cleaning motion detected");
    had_motion_detected = false;
    last_motion_detected = 0;

    // turn off the light
    is_automatic_bright = false;
    led_bright = 0;
  }
  // --- end PIR sensor

}


// Define the led bright automatically
void autoDefineLEDBright() {
    int analogValue = analogRead(PIN_TO_LIGHT_SENSOR);
    led_bright = map(analogValue, 0, 4095, 1024, 0);
    ledcWrite(led_pwm_channel, led_bright);
    //Serial.print("Analog Value: ");
    //Serial.println(analogValue);
}
