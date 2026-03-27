#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t mtx;

static void print_block(const char *tag) {
  for (int i = 0; i < 5; i++) {
    Serial.printf("[%s] line=%d\n", tag, i);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void TaskA(void *pv) {
  (void)pv;
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(5));
    if(xSemaphoreTake(mtx, portMAX_DELAY)) {    
      print_block("A");
      xSemaphoreGive(mtx);
    }  
}
}

void TaskB(void *pv) {
  (void)pv;
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(5));
    if (xSemaphoreTake(mtx, portMAX_DELAY) == pdTRUE) {
      print_block("B");
      xSemaphoreGive(mtx);
    }
  }
}

void setup() {
  Serial.begin(115200);

  mtx = xSemaphoreCreateMutex();
  if (!mtx) {
    Serial.println("Falha ao criar mutex!");
    while (true) { delay(1000); }
  }

  xTaskCreate(TaskA, "A", 2048, nullptr, 2, nullptr);
  xTaskCreate(TaskB, "B", 2048, nullptr, 1, nullptr);
}

void loop() {}
