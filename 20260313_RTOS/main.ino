#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void TaskHigh(void *pv){
  while(true){
    Serial.println("[HIGH] prio = 2 - running");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void TaskLow(void *pv){
  while(true){
    Serial.println("[LOW] prio = 1 - running");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);

  xTaskCreate(TaskLow, "LOW", 2048, nullptr, 1, nullptr);
  xTaskCreate(TaskHigh, "High", 2048, nullptr, 2, nullptr);
}

void loop() {
  
}

// Link do Wokwi: https://wokwi.com/projects/458420949339994113
