#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t q;

void Producer(void *pv){
  int v = 0;
  while(true){
    if(xQueueSend(q, &v, pdMS_TO_TICKS(50)) == pdPASS){
      Serial.printf("[Producer] data sent = %d\n", v);
      v++;
    } else {
      Serial.println("[Producer] fail to send data");
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void Consumer(void *pv){
  int v;
  while(true){
    if(xQueueReceive(q, &v, portMAX_DELAY) == pdPASS){
      Serial.printf("[Consumer] data received = %d\n", v);
    }
  }
}

void setup() {
  Serial.begin(115200);

  q = xQueueCreate(1, sizeof(int));
  if(!q){
    Serial.println("Falha ao criar fila!");
    while(true) {}
  }

  xTaskCreate(Producer, "Producer", 2048, nullptr, 1, nullptr);
  xTaskCreate(Consumer, "Consumer", 2048, nullptr, 1, nullptr);
}

void loop() {
  // vTaskDelay(pdMS_TO_TICKS(1000));
}

// Link do Wokwi: https://wokwi.com/projects/459053561244233729
