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
const int PIN_TO_LIGHT_SENSOR = 25;
const int PIN_TO_LED = 26;
const int PIN_TO_BUTTON = 33;

// variables
bool is_automatic_bright = true;
int led_bright = 0;

int button_state = HIGH;
int last_button_state = HIGH;

// delay control
unsigned long current_time;
unsigned long last_time = 200;


// pwd setup
const int led_pwm_frequency = 9000;
const int led_pwm_channel = 0;
const int led_pwm_resolution = 10;


// setup API routes
void setup_routing() {

    // app root page
    server.on("/", HTTP_GET, []() {
      server.send(200, "text/html","<!DOCTYPE html><html lang='pt-BR'><head> <meta charset='utf-8'> <meta name='viewport' content='width=device-width, initial-scale=1'> <title>Smart Light</title></head><body> <section class='section'> <div class='container'> <h1 class='title'> Smart Light </h1> <p class='subtitle'> Controle sua lâmpada aqui! </p></div></section> <section class='section'> <div class='container'> <form> <div class='field'> <label class='label'>Luminosidade:</label> <div class='control'> <input id='lumi' class='input' type='number' max='1024' min='0' placeholder='Ex.: 50' onchange='onChangeLuminosityValue'/> </div></div><div class='field is-grouped'> <div class='control'> <button class='button is-success' type='button' onclick='sendChangeLuminosity()'> Enviar </button> </div></div><div class='is-divider'></div><div class='field'> <label class='label'>Ativar LDR:</label> <div class='control'> <label class='switch'> <input type='checkbox'> <span class='slider round'></span> </label> </div></div><div class='field is-grouped'> <div class='control'> <button class='button is-success' type='button' onclick='sendEnableLDR()'> Enviar </button> </div></div><div class='is-divider'></div></form> </div></section></body><script>function onChangeLuminosityValue(){console.log('safado');}async function sendEnableLDR(){const url='http://192.168.1.1/led-auto-bright'; const options={method: 'POST', mode: 'no-cors', cache: 'no-cache', body: luminosityInput.value,}; const response=await fetch(url, options);}async function sendChangeLuminosity(){const luminosityInput=document.getElementById('lumi'); if (!luminosityInput){return;}if (luminosityInput.value > 1024){alert('O valor de ser entre 0 e 1024'); return;}const url='http://192.168.1.1/led-bright'; const options={method: 'POST', mode: 'no-cors', cache: 'no-cache', body: luminosityInput.value,}; const response=await fetch(url, options); console.log(response);}</script><style>body,button,input,optgroup,select,textarea{font-family: BlinkMacSystemFont, -apple-system, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', 'Fira Sans', 'Droid Sans', 'Helvetica Neue', 'Helvetica', 'Arial', sans-serif;}.switch{position: relative; display: inline-block; width: 60px; height: 34px;}.switch input{opacity: 0; width: 0; height: 0;}.slider{position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}.slider:before{position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}input:checked + .slider{background-color: #2196F3;}input:focus + .slider{box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before{-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}.slider.round{border-radius: 34px;}.slider.round:before{border-radius: 50%;}</style></html>");
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


}


void setup() {
  Serial.begin(115200);

  // setup pins
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_BUTTON, INPUT_PULLUP);

  // setup pwm
  ledcSetup(led_pwm_channel, led_pwm_frequency, led_pwm_resolution);
  ledcAttachPin(PIN_TO_LED, led_pwm_channel);


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

}


// Define the led bright automatically
void autoDefineLEDBright() {
    int analogValue = analogRead(PIN_TO_LIGHT_SENSOR);
    led_bright = map(analogValue, 0, 4095, 1024, 0);
    ledcWrite(led_pwm_channel, led_bright);
    //Serial.print("Analog Value: ");
    //Serial.println(analogValue);
}
