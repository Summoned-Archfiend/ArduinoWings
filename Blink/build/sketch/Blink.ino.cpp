#include <Arduino.h>
#line 1 "/home/lukemccann/Projects/ArduinoWings/Blink/Blink.ino"
#line 1 "/home/lukemccann/Projects/ArduinoWings/Blink/Blink.ino"
void setup();
#line 6 "/home/lukemccann/Projects/ArduinoWings/Blink/Blink.ino"
void loop();
#line 1 "/home/lukemccann/Projects/ArduinoWings/Blink/Blink.ino"
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  Serial.println("blink");
}

