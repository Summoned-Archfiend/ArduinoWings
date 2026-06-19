// ButtonMotorTest - test the button + motor driver + motors together, with NO
// limit switches wired yet. The button (D2) toggles the motor:
//
//   tap -> OPEN (runs)  ->  tap -> STOP  ->  tap -> CLOSE (runs)  ->  tap -> STOP  -> ...
//
// You are the limit switch here: tap again to stop whenever you've seen enough.
// There are no end-stops, so the motor runs until you tap or the safety timeout
// trips. Keep a hand on the 12V disconnect.
//
// Wiring (matches WingControl, minus the limit switches):
//   Button:  one leg -> D2,  other leg -> GND   (polarity doesn't matter)
//   Driver:  D5 RPWM, D6 LPWM, D7 R_EN, D8 L_EN, VCC -> 5V, GND -> common gnd
//   Motors:  on the driver's M+/M- (already wired)

const int BUTTON_PIN = 2;
const int RPWM = 5;
const int LPWM = 6;
const int R_EN = 7;
const int L_EN = 8;

// Gentle to start. Raise toward 200 if the worm gear won't move at this speed.
const int TEST_SPEED = 150;   // 0-255

// Safety backstop: with no limit switches, auto-stop after this long so a run
// can't continue forever if you step away. Tap the button to stop sooner.
const unsigned long MOVE_TIMEOUT_MS = 15000;

const unsigned long DEBOUNCE_MS = 30;

// ---- State ---------------------------------------------------------------
enum State { IDLE, OPENING, CLOSING };
State state = IDLE;
bool lastWasOpen = false;        // so the first tap from IDLE opens
unsigned long moveStart = 0;

// ---- Motor ---------------------------------------------------------------
void motorStop() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  digitalWrite(R_EN, LOW);       // disable both bridges when idle
  digitalWrite(L_EN, LOW);
}

void motorOpen() {               // RPWM direction (matches WingControl)
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(LPWM, 0);
  analogWrite(RPWM, TEST_SPEED);
}

void motorClose() {              // LPWM direction
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, TEST_SPEED);
}

// ---- Button (debounced press edge) ---------------------------------------
int btnReading = HIGH;
int btnStable  = HIGH;
unsigned long btnChangedAt = 0;

bool buttonPressEdge() {
  bool edge = false;
  const int reading = digitalRead(BUTTON_PIN);
  if (reading != btnReading) { btnChangedAt = millis(); btnReading = reading; }
  if (millis() - btnChangedAt > DEBOUNCE_MS && reading != btnStable) {
    btnStable = reading;
    if (btnStable == LOW) edge = true;   // pressed = LOW (INPUT_PULLUP)
  }
  return edge;
}

// ---- Setup ---------------------------------------------------------------
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  motorStop();
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { }
  Serial.println(F("ButtonMotorTest ready - tap button: OPEN / STOP / CLOSE / STOP ..."));
}

// ---- Loop ----------------------------------------------------------------
void loop() {
  const bool pressed = buttonPressEdge();

  switch (state) {
    case IDLE:
      if (pressed) {
        if (lastWasOpen) { state = CLOSING; motorClose(); Serial.println(F("CLOSING")); }
        else             { state = OPENING; motorOpen();  Serial.println(F("OPENING")); }
        lastWasOpen = !lastWasOpen;
        moveStart = millis();
      }
      break;

    case OPENING:
    case CLOSING:
      if (pressed) {
        motorStop(); state = IDLE; Serial.println(F("STOP (button)"));
      } else if (millis() - moveStart > MOVE_TIMEOUT_MS) {
        motorStop(); state = IDLE; Serial.println(F("STOP (timeout)"));
      }
      break;
  }
}
