// SystemTest - exercise EVERYTHING together before trusting WingControl:
// button (D2), open limit (D3), closed limit (D4), and the motor driver/motors.
// Simpler than WingControl on purpose - no FAULT lockout, no boot-position
// logic. Just a functional check that every part responds.
//
// Button toggles the motor:
//   tap -> OPEN (runs) -> tap -> STOP -> tap -> CLOSE (runs) -> tap -> STOP -> ...
// While moving, the matching limit switch stops the motor automatically:
//   opening + open limit (D3) triggered  -> STOP
//   closing + closed limit (D4) triggered -> STOP
// Every input also prints live whenever it changes, and the onboard LED lights
// while any input is active - so you can press the limits/button by hand (even
// off the wing) and watch them register.
//
// Wiring (matches WingControl):
//   Button:        leg -> D2,  leg -> GND
//   Open limit:    C -> GND,   NO -> D3
//   Closed limit:  C -> GND,   NO -> D4
//   Driver:        D5 RPWM, D6 LPWM, D7 R_EN, D8 L_EN, VCC -> 5V, GND -> gnd
//   Motors:        on driver M+/M-

const int BUTTON_PIN       = 2;
const int OPEN_LIMIT_PIN   = 3;
const int CLOSED_LIMIT_PIN = 4;
const int RPWM = 5;
const int LPWM = 6;
const int R_EN = 7;
const int L_EN = 8;

const int TEST_SPEED = 150;                  // 0-255, raise toward 200 if needed
const unsigned long MOVE_TIMEOUT_MS = 15000; // safety backstop while moving
const unsigned long DEBOUNCE_MS     = 30;

// ---- State ---------------------------------------------------------------
enum State { IDLE, OPENING, CLOSING };
State state = IDLE;
bool lastWasOpen = false;        // first tap from IDLE opens
unsigned long moveStart = 0;

// ---- Motor ---------------------------------------------------------------
void motorStop() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  digitalWrite(R_EN, LOW);
  digitalWrite(L_EN, LOW);
}
void motorOpen() {
  digitalWrite(R_EN, HIGH); digitalWrite(L_EN, HIGH);
  analogWrite(LPWM, 0); analogWrite(RPWM, TEST_SPEED);
}
void motorClose() {
  digitalWrite(R_EN, HIGH); digitalWrite(L_EN, HIGH);
  analogWrite(RPWM, 0); analogWrite(LPWM, TEST_SPEED);
}

// ---- Inputs ---------------------------------------------------------------
bool openLimitHit()   { return digitalRead(OPEN_LIMIT_PIN)   == LOW; }
bool closedLimitHit() { return digitalRead(CLOSED_LIMIT_PIN) == LOW; }

int btnReading = HIGH, btnStable = HIGH;
unsigned long btnChangedAt = 0;
bool buttonPressEdge() {
  bool edge = false;
  const int reading = digitalRead(BUTTON_PIN);
  if (reading != btnReading) { btnChangedAt = millis(); btnReading = reading; }
  if (millis() - btnChangedAt > DEBOUNCE_MS && reading != btnStable) {
    btnStable = reading;
    if (btnStable == LOW) edge = true;
  }
  return edge;
}

// Live input reporting (so you can press each input by hand and see it).
int lastBtn = HIGH, lastOpen = HIGH, lastClosed = HIGH;
void reportInputs() {
  const int b = digitalRead(BUTTON_PIN);
  const int o = digitalRead(OPEN_LIMIT_PIN);
  const int c = digitalRead(CLOSED_LIMIT_PIN);
  digitalWrite(LED_BUILTIN, (b == LOW || o == LOW || c == LOW) ? HIGH : LOW);
  if (b != lastBtn)    { Serial.println(b == LOW ? F("  BUTTON       PRESSED")   : F("  BUTTON       released"));   lastBtn = b; }
  if (o != lastOpen)   { Serial.println(o == LOW ? F("  OPEN LIMIT   TRIGGERED") : F("  OPEN LIMIT   open"));       lastOpen = o; }
  if (c != lastClosed) { Serial.println(c == LOW ? F("  CLOSED LIMIT TRIGGERED") : F("  CLOSED LIMIT open"));       lastClosed = c; }
}

// ---- Setup ---------------------------------------------------------------
void setup() {
  pinMode(BUTTON_PIN,       INPUT_PULLUP);
  pinMode(OPEN_LIMIT_PIN,   INPUT_PULLUP);
  pinMode(CLOSED_LIMIT_PIN, INPUT_PULLUP);
  pinMode(RPWM, OUTPUT); pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT); pinMode(L_EN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  motorStop();
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { }
  Serial.println(F("SystemTest ready."));
  Serial.println(F("Tap button: OPEN/STOP/CLOSE/STOP. Limits stop the motor mid-move."));
  Serial.println(F("Press button/limits by hand to see them register below:"));
}

// ---- Loop ----------------------------------------------------------------
void loop() {
  reportInputs();
  const bool pressed = buttonPressEdge();

  switch (state) {
    case IDLE:
      if (pressed) {
        if (lastWasOpen) { state = CLOSING; motorClose(); Serial.println(F("-> CLOSING")); }
        else             { state = OPENING; motorOpen();  Serial.println(F("-> OPENING")); }
        lastWasOpen = !lastWasOpen;
        moveStart = millis();
      }
      break;

    case OPENING:
      if (openLimitHit())      { motorStop(); state = IDLE; Serial.println(F("-> STOP (open limit reached)")); }
      else if (pressed)        { motorStop(); state = IDLE; Serial.println(F("-> STOP (button)")); }
      else if (millis() - moveStart > MOVE_TIMEOUT_MS) { motorStop(); state = IDLE; Serial.println(F("-> STOP (timeout)")); }
      break;

    case CLOSING:
      if (closedLimitHit())    { motorStop(); state = IDLE; Serial.println(F("-> STOP (closed limit reached)")); }
      else if (pressed)        { motorStop(); state = IDLE; Serial.println(F("-> STOP (button)")); }
      else if (millis() - moveStart > MOVE_TIMEOUT_MS) { motorStop(); state = IDLE; Serial.println(F("-> STOP (timeout)")); }
      break;
  }
}
