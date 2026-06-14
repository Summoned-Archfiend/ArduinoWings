const int openCloseButtonPin = 2;
const int ledPin = 12;

int lastState = HIGH;

void setup() {
  pinMode(openCloseButtonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Button check ready - press the button"));
}

void loop() {
  const int state = digitalRead(openCloseButtonPin);

  // Both LEDs on while button held (LOW = pressed).
  digitalWrite(ledPin, state == LOW ? HIGH : LOW);
  digitalWrite(LED_BUILTIN, state == LOW ? HIGH : LOW);

  // Print only when the state changes, so the monitor isn't flooded.
  if (state != lastState) {
    Serial.println(state == LOW ? F("PRESSED  (D2 = LOW)") : F("released (D2 = HIGH)"));
    lastState = state;
  }
}
