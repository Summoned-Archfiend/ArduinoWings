// LimitTest - check all three logic inputs: button (D2), open limit (D3),
// closed limit (D4). All use INPUT_PULLUP, so triggered/pressed = LOW. Each
// input prints its own labelled line on change; the on-board LED lights
// whenever ANY input is active.
//
// Wiring (matches WingControl):
//   Button:             one leg -> D2,  other leg -> GND
//   Open limit switch:   C -> GND,      NO -> D3
//   Closed limit switch: C -> GND,      NO -> D4
//   Not pressed = HIGH.  Pressed/triggered = LOW.

const int buttonPin      = 2;
const int openLimitPin   = 3;
const int closedLimitPin = 4;

int lastButton = HIGH;
int lastOpen   = HIGH;
int lastClosed = HIGH;

void setup() {
  pinMode(buttonPin,      INPUT_PULLUP);
  pinMode(openLimitPin,   INPUT_PULLUP);
  pinMode(closedLimitPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN,    OUTPUT);
  Serial.begin(115200);
  Serial.println(F("LimitTest ready - press button (D2), open limit (D3), closed limit (D4)"));
}

void loop() {
  const int button = digitalRead(buttonPin);
  const int openSw = digitalRead(openLimitPin);
  const int closed = digitalRead(closedLimitPin);

  // On-board LED on if any input is active (LOW).
  digitalWrite(LED_BUILTIN, (button == LOW || openSw == LOW || closed == LOW) ? HIGH : LOW);

  // Print each input only when it changes, with its own label.
  if (button != lastButton) {
    Serial.println(button == LOW ? F("BUTTON        PRESSED   (D2 = LOW)")
                                 : F("BUTTON        released  (D2 = HIGH)"));
    lastButton = button;
  }

  if (openSw != lastOpen) {
    Serial.println(openSw == LOW ? F("OPEN LIMIT    TRIGGERED (D3 = LOW)")
                                 : F("OPEN LIMIT    open      (D3 = HIGH)"));
    lastOpen = openSw;
  }

  if (closed != lastClosed) {
    Serial.println(closed == LOW ? F("CLOSED LIMIT  TRIGGERED (D4 = LOW)")
                                 : F("CLOSED LIMIT  open      (D4 = HIGH)"));
    lastClosed = closed;
  }
}
