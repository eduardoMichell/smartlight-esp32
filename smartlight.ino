//PINS
const int PIN_TO_SENSOR = 19;
const int PIN_TO_LED = 27;

// STATES
int pinStateCurrent   = LOW;  // current state of pin
int pinStatePrevious  = LOW;  // previous state of pin

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_SENSOR, INPUT);
}

void loop() {
  pinStatePrevious = pinStateCurrent;
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
    Serial.println("Motion detected!");
    digitalWrite(PIN_TO_LED, HIGH);
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {
    Serial.println("Motion stopped!");
    digitalWrite(PIN_TO_LED, LOW);
  }
}
