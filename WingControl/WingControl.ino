// WingControl - both wings move together from a SINGLE BTS7960 driver.
// The two motors are wired in PARALLEL on the driver's M+/M- (one motor's leads
// reversed so both wings open the same way). To the firmware this is just one
// motor, so no per-wing logic - D5-D8 drive the single driver as normal.
// The two open-limit switches are paralleled onto D3, the two closed-limit
// switches onto D4 (either one triggering = that end reached). See CIRCUIT.md.

const int BUTTON_PIN       = 2;
const int OPEN_LIMIT_PIN   = 3;
const int CLOSED_LIMIT_PIN = 4;
const int RPWM = 5;
const int LPWM = 6;
const int R_EN = 7;
const int L_EN = 8;

// ---- Tuning --------------------------------------------------------------
const int OPEN_SPEED  = 200;   // 0-255 PWM. Worm gear is slow; raise if needed.
const int CLOSE_SPEED = 200;

// Backstop only. Must be LONGER than the worst-case real travel time.
// At 6 RPM a full open can take 20-30s - measure yours and add margin.
const unsigned long MOVE_TIMEOUT_MS = 30000;

const unsigned long DEBOUNCE_MS         = 30;
const unsigned long FAULT_RESET_HOLD_MS = 1500;  // hold button to clear a FAULT

// ---- State ---------------------------------------------------------------
enum WingState { CLOSED, OPENING, OPEN, CLOSING, FAULT };
WingState state;
unsigned long moveStart = 0;

const char* stateName(WingState s) {
  switch (s) {
    case CLOSED:  return "CLOSED";
    case OPENING: return "OPENING";
    case OPEN:    return "OPEN";
    case CLOSING: return "CLOSING";
    case FAULT:   return "FAULT";
  }
  return "?";
}

void enter(WingState s) {
  state = s;
  Serial.print(F("state -> "));
  Serial.println(stateName(s));
}

// ---- Button (debounced edge + hold detection) ----------------------------
int btnReading = HIGH;
int btnStable  = HIGH;
unsigned long btnChangedAt = 0;
unsigned long btnPressedAt = 0;

// Call once per loop. Returns true exactly once on the press edge.
bool buttonPressEdge() {
  bool edge = false;
  const int reading = digitalRead(BUTTON_PIN);
  if (reading != btnReading) { btnChangedAt = millis(); btnReading = reading; }
  if (millis() - btnChangedAt > DEBOUNCE_MS && reading != btnStable) {
    btnStable = reading;
    if (btnStable == LOW) { edge = true; btnPressedAt = millis(); }
  }
  return edge;
}

bool buttonHeldFor(unsigned long ms) {
  return btnStable == LOW && (millis() - btnPressedAt >= ms);
}

// ---- Limit switches (active LOW via INPUT_PULLUP) ------------------------
bool openLimitHit()   { return digitalRead(OPEN_LIMIT_PIN)   == LOW; }
bool closedLimitHit() { return digitalRead(CLOSED_LIMIT_PIN) == LOW; }

// ---- Motor ---------------------------------------------------------------
void motorStop() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  digitalWrite(R_EN, LOW);   // fully disable bridges when idle (worm gear holds)
  digitalWrite(L_EN, LOW);
}

void motorOpen() {           // winds cable -> wing opens
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(LPWM, 0);
  analogWrite(RPWM, OPEN_SPEED);
}

void motorClose() {          // pays cable out -> return spring folds wing
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, CLOSE_SPEED);
}

// ---- Setup ---------------------------------------------------------------
void setup() {
  pinMode(BUTTON_PIN,       INPUT_PULLUP);
  pinMode(OPEN_LIMIT_PIN,   INPUT_PULLUP);
  pinMode(CLOSED_LIMIT_PIN, INPUT_PULLUP);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  motorStop();
  Serial.begin(115200);
  // Native-USB board: give the serial monitor up to 3s to attach so the boot
  // line is visible. Times out so it still runs fine on battery with no PC.
  while (!Serial && millis() < 3000) { }

  // We don't know the wing position at power-on - infer it from the limits.
  // If neither is pressed (wing part-way), refuse to guess: FAULT and wait.
  if (closedLimitHit())      enter(CLOSED);
  else if (openLimitHit())   enter(OPEN);
  else {
    enter(FAULT);
    Serial.println(F("Unknown position at boot - move wing to a limit, hold button to reset"));
  }
}

// ---- Loop ----------------------------------------------------------------
void loop() {
  // Whenever the serial monitor (re)connects, print the current state so you
  // never get a blank monitor - handles the native-USB "missed boot" problem.
  static bool wasConnected = false;
  const bool isConnected = (bool) Serial;
  if (isConnected && !wasConnected) {
    Serial.print(F("monitor connected, current state = "));
    Serial.println(stateName(state));
  }
  wasConnected = isConnected;

  const bool pressed = buttonPressEdge();

  // Wiring sanity check: both limits at once is impossible -> fault out.
  if (state != FAULT && openLimitHit() && closedLimitHit()) {
    motorStop();
    enter(FAULT);
    Serial.println(F("Both limits active - check wiring"));
    return;
  }

  switch (state) {
    case CLOSED:
      if (pressed) { enter(OPENING); moveStart = millis(); motorOpen(); }
      break;

    case OPENING:
      if (openLimitHit())                       { motorStop(); enter(OPEN); }
      else if (pressed)                         { motorStop(); enter(FAULT); Serial.println(F("emergency stop")); }
      else if (millis() - moveStart > MOVE_TIMEOUT_MS) { motorStop(); enter(FAULT); Serial.println(F("open timeout")); }
      break;

    case OPEN:
      if (pressed) { enter(CLOSING); moveStart = millis(); motorClose(); }
      break;

    case CLOSING:
      if (closedLimitHit())                     { motorStop(); enter(CLOSED); }
      else if (pressed)                         { motorStop(); enter(FAULT); Serial.println(F("emergency stop")); }
      else if (millis() - moveStart > MOVE_TIMEOUT_MS) { motorStop(); enter(FAULT); Serial.println(F("close timeout")); }
      break;

    case FAULT:
      motorStop();
      // Recover only from a known position: hold the button, and we adopt
      // whichever limit is currently pressed. Otherwise stay faulted.
      if (buttonHeldFor(FAULT_RESET_HOLD_MS)) {
        if (closedLimitHit())    { enter(CLOSED); }
        else if (openLimitHit()) { enter(OPEN); }
        else { Serial.println(F("still no limit - move wing to a hard stop first")); btnPressedAt = millis(); }
      }
      break;
  }
}
