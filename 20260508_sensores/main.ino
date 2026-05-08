#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct {
  float temperature;
  unsigned long timestamp;
} SensorData;

QueueHandle_t sensorqueue;
QueueHandle_t logqueue;

SemaphoreHandle_t sem;

SemaphoreHandle_t mutex;

void TaskSensor (void *pv){
  SensorData data;
  while(1){
    data.temperature = random(200, 350);
    data.timestamp = millis();

    xQueueSend(sensorqueue, &data, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(1000)); 
  }
}

void TaskProcessing (void *pv){
  SensorData data;
  while(1){
    if(xQueueReceive(sensorqueue, &data, portMAX_DELAY) == pdTRUE){
      data.temperature = data.temperature/10;
      xQueueSend(logqueue, &data, portMAX_DELAY);
      xSemaphoreGive(sem);
    }
  }
}

void TaskLog (void *pv){
  SensorData data;
  while(1){
    if(xSemaphoreTake(sem, portMAX_DELAY) == pdTRUE){
      if(xQueueReceive(logqueue, &data, portMAX_DELAY) == pdTRUE){
        xSemaphoreTake(mutex, portMAX_DELAY);

        Serial.print("Temperatura: ");
        Serial.print(data.temperature);
        Serial.print("| Timestamp: ");
        Serial.print(data.timestamp);
        Serial.print("\n");

        xSemaphoreGive(mutex);
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  sensorqueue = xQueueCreate(2, sizeof(SensorData));
  logqueue = xQueueCreate(2, sizeof(SensorData));

  sem = xSemaphoreCreateBinary();

  mutex = xSemaphoreCreateMutex();

  xTaskCreate(TaskSensor, "Sensor", (2*1024), NULL, 2, NULL);
  xTaskCreate(TaskProcessing, "Processing", (2*1024), NULL, 2, NULL);
  xTaskCreate(TaskLog, "Log", (4*1024), NULL, 1, NULL);
}

void loop() {
}

// Wokwi: https://wokwi.com/projects/463495341338066945
