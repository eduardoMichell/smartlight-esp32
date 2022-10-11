//PINS
const int PIN_TO_SENSOR = 26;
const int PIN_TO_LED = 27;
const int PIN_TO_LIGHT_SENSOR = 25;

// STATES
int pinStateCurrent   = LOW;  // current state of pin
int pinStatePrevious  = LOW;  // previous state of pin


const int freq = 9000;
const int ledChannel = 0;
const int resolution = 10;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TO_LED, OUTPUT);
  pinMode(PIN_TO_SENSOR, INPUT);


  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PIN_TO_LED, ledChannel);
}

void loop() {
  pinStatePrevious = pinStateCurrent;
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
    Serial.println("Motion detected!");
    digitalWrite(PIN_TO_LED, HIGH);
    calculateBright(1);
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {
    Serial.println("Motion stopped!");
    digitalWrite(PIN_TO_LED, LOW);
    calculateBright(0);
  }  
}


void calculateBright(int state) {
  if (state == 1) {
    // (value between 0 and 4095)
    int analogValue = analogRead(PIN_TO_LIGHT_SENSOR);
    Serial.print("Analog Value = ");
    Serial.print(analogValue);
    if (analogValue < 40) {
      Serial.println(" => Dark");
      ledcWrite(ledChannel, 255);
    } else if (analogValue < 800) {
      Serial.println(" => Dim");
      ledcWrite(ledChannel, 200);
    } else if (analogValue < 2000) {
      Serial.println(" => Light");
      ledcWrite(ledChannel, 150);
    } else if (analogValue < 3200) {
      Serial.println(" => Bright");
      ledcWrite(ledChannel, 100);
    } else {
      Serial.println(" => Very bright");
      ledcWrite(ledChannel, 0);
    }
  } else {
    ledcWrite(ledChannel, 0);
  }
}
