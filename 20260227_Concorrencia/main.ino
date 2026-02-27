// Aula de concorrencia
// Link do Projeto: https://wokwi.com/projects/457154225465048065
#include <Arduino.h>

static const int LED_PIN = 2;
static const int BTN_PIN = 4;

// Estado do &Comodo&
static bool fastMode = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
}

void led_timing(bool mode, unsigned long now) {
  static unsigned long lastLed = 0;
  unsigned long ledPeriod = mode ? 80 : 200;

  if(now - lastLed >= ledPeriod){
    lastLed = now;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void log_alive(unsigned long now) {
  static unsigned long lastLog = 0;

  if (now - lastLog >= 1000){
    lastLog = now;
    Serial.printf("[loop] modo = %s\n", fastMode ? "FAST" : "NORMAL");
  }
}

void button_press(){
  if(digitalRead(BTN_PIN) == LOW){
    delay(50);
    if(digitalRead(BTN_PIN) == LOW){
      fastMode = !fastMode;
      Serial.printf("[btn] modo = %s\n", fastMode ? "FAST" : "NORMAL");
      while(digitalRead(BTN_PIN) == LOW) {}
    }
    delay(50);
  }
}

void loop() {
  unsigned long now = millis();
  button_press();
  led_timing(fastMode, now);
  log_alive(now);
}
