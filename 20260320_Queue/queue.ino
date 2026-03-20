#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t sensorQueue;

typedef struct {
  float temp;
  float hum;
  long unsigned int timestamp;
} SensorData;

void SensorTask(void *pv) {
    while (1) {
      SensorData data;

      data.temp = random(20, 30);
      data.hum = random(0, 100);
      data.timestamp = millis();

      xQueueSend(sensorQueue, &data, portMAX_DELAY);
	    vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ProcessingTask(void *pv) {
    SensorData received;

    while (1) {
        if (xQueueReceive(sensorQueue, &received, portMAX_DELAY)) {
          Serial.printf("[ProcessingTask] Temp = %.2f | Hum = %.2f | Timestamp = %lu\n", received.temp, received.hum, received.timestamp);
        }
    }
}

void setup() {
    Serial.begin(115200);

    sensorQueue = xQueueCreate(2, sizeof(SensorData));
    if(!sensorQueue){
      Serial.println("Falha ao criar fila!");
      while(true) {}
    }

    xTaskCreate(SensorTask, "Sensor", 2048, NULL, 1, NULL);
    xTaskCreate(ProcessingTask, "Processing", 2048, NULL, 1, NULL);
}

void loop() {}


// Link do Wokwi: https://wokwi.com/projects/459054686606512129
