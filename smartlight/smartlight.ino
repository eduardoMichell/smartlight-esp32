const int PIN_TO_SENSOR = 27;
const int PIN_TO_LED = 26;
const int PIN_TO_LIGHT_SENSOR = 25;
const int PIN_TO_BUTTON = 18;

int pinStateCurrent   = LOW; 
int pinStatePrevious  = LOW; 
int lastButtonState = HIGH; 
int currentButtonState;     
int buttonState = LOW;     

const int freq = 9000;
const int ledChannel = 0;
const int resolution = 10;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_SENSOR, INPUT);
  pinMode(PIN_TO_BUTTON, INPUT_PULLUP);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PIN_TO_LED, ledChannel);
}

void loop() {
  currentButtonState = digitalRead(PIN_TO_BUTTON);
  if (lastButtonState == LOW && currentButtonState == HIGH) {
    buttonState = !buttonState;
    Serial.print("Button Pressed: ");
    Serial.println(buttonState);
  }

  lastButtonState = currentButtonState;
  if (buttonState == HIGH) {
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
