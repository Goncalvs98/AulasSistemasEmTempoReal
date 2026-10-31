#include <Arduino.h>

static const int LED_PIN = 2;
bool led = false;
unsigned long last = 0;
unsigned long lastPrint = 0;

// Tarefa 1: Piscar um led
void blink_led(){
  unsigned long now = millis();
  unsigned long diff = now - last;

  if(diff >= 500){
    digitalWrite(LED_PIN, led);
    led = !led;
    last = now;

    Serial.printf("[milis] periodo = %lu ms\n", diff);
  }
}

void process_anything(){
  unsigned long temp0 = micros();
  for(volatile long i = 0; i < 5000; i++){}
  unsigned long temp1 = micros() - temp0;
  if(millis()-lastPrint >= 2000){
    Serial.printf("[micros] periodo = %lu us\n", temp1);
    lastPrint = millis();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inicio!");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  blink_led();
  process_anything();
}
