// MotorTest - bench test the BTS7960 + JGY-370 motor. NO WING/CABLE ATTACHED.
// Arduino stays on USB; motor is powered by the 12V battery via BTS7960 B+/B-.
// Common ground required: Arduino GND <-> BTS7960 GND <-> battery -.
//
// SAFETY: secure the motor, keep a hand on the 12V disconnect, free shaft only.
//
// Pins (match PLAN.md):
//   D5 RPWM, D6 LPWM  (speed - must be PWM '~' pins)
//   D7 R_EN, D8 L_EN  (enable - on/off)

const int RPWM = 5;
const int LPWM = 6;
const int R_EN = 7;
const int L_EN = 8;

// Start gentle. Raise later only if the motor won't move (worm gears are slow).
const int TEST_SPEED = 90;   // 0-255

void stopMotor() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void runForward(int speed) {
  analogWrite(LPWM, 0);
  analogWrite(RPWM, speed);
}

void runReverse(int speed) {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, speed);
}

void setup() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);   // enable both half-bridges
  digitalWrite(L_EN, HIGH);

  stopMotor();
  Serial.begin(115200);
  Serial.println(F("MotorTest: starting in 3s - keep hand on 12V disconnect"));
  delay(3000);                // grace period before first movement
}

void loop() {
  Serial.println(F("FORWARD"));
  runForward(TEST_SPEED);
  delay(2500);

  Serial.println(F("stop"));
  stopMotor();
  delay(1500);

  Serial.println(F("REVERSE"));
  runReverse(TEST_SPEED);
  delay(2500);

  Serial.println(F("stop"));
  stopMotor();
  delay(1500);
}
