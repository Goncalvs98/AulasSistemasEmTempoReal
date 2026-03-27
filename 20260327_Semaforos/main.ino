// https://wokwi.com/projects/459687242643275777
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t semA;
SemaphoreHandle_t semB;
SemaphoreHandle_t semC;

static void print_block(const char *tag) {
  for (int i = 0; i < 5; i++) {
    Serial.printf("[%s] line=%d\n", tag, i);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void TaskA(void *pv) {
  (void)pv;

  while (true) {
    if (xSemaphoreTake(semA, portMAX_DELAY) == pdTRUE) {
      print_block("A");
      xSemaphoreGive(semB);
    }
  }
}

void TaskB(void *pv) {
  (void)pv;

  while (true) {
    if (xSemaphoreTake(semB, portMAX_DELAY) == pdTRUE) {
      print_block("B");
      xSemaphoreGive(semC);
    }
  }
}

void TaskC(void *pv) {
  (void)pv;

  while (true) {
    if (xSemaphoreTake(semC, portMAX_DELAY) == pdTRUE) {
      print_block("C");
      xSemaphoreGive(semA);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  semA = xSemaphoreCreateBinary();
  semB = xSemaphoreCreateBinary();
  semC = xSemaphoreCreateBinary();

  if (!semA || !semB || !semC) {
    Serial.println("Falha ao criar semaforos!");
    while (true) {}
  }

  xTaskCreate(TaskA, "TaskA", 2048, nullptr, 3, nullptr);
  xTaskCreate(TaskB, "TaskB", 2048, nullptr, 2, nullptr);
  xTaskCreate(TaskC, "TaskC", 2048, nullptr, 1, nullptr);

  xSemaphoreGive(semA);
}

void loop() {}
