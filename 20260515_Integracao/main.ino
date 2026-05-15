#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#define BTN_MODE_PIN     4
#define BTN_DISPLAY_PIN  5
#define LED_PIN          2

typedef enum {
  EVENT_MODE_BUTTON,
  EVENT_DISPLAY_BUTTON
} EventType;

typedef struct {
  EventType type;
  unsigned long timestamp;
} ButtonEvent;

QueueHandle_t buttonQueue;

SemaphoreHandle_t semDisplaySpecial;

SemaphoreHandle_t mtxDisplay;
SemaphoreHandle_t mtxSerial;
SemaphoreHandle_t mtxMode;

int systemMode = 0;

void TaskButtons(void *pv) {
  ButtonEvent event;

  int lastBtnMode = HIGH;
  int lastBtnDisplay = HIGH;
  while (true) {
    int btnMode = digitalRead(BTN_MODE_PIN);
    int btnDisplay = digitalRead(BTN_DISPLAY_PIN);

    if(lastBtnMode == HIGH && btnMode == LOW){
      event.type = EVENT_MODE_BUTTON;
      event.timestamp = millis();
      xQueueSend(buttonQueue, &event, portMAX_DELAY);
    }

    if(lastBtnDisplay == HIGH && btnDisplay == LOW){
      event.type = EVENT_DISPLAY_BUTTON;
      event.timestamp = millis();
      xQueueSend(buttonQueue, &event, portMAX_DELAY);
    }

    lastBtnMode = btnMode;
    lastBtnDisplay = btnDisplay;

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void TaskControl(void *pv) {
  ButtonEvent received;
  while (true) {
    if(xQueueReceive(buttonQueue, &received, portMAX_DELAY)){
      if(received.type == EVENT_MODE_BUTTON){
        xSemaphoreTake(mtxMode, portMAX_DELAY);
        systemMode = !systemMode;
        int mode = systemMode;

        xSemaphoreGive(mtxMode);

        xSemaphoreTake(mtxSerial, portMAX_DELAY);
        Serial.print("[CONTROL] Botão 1 foi pressionado. Modo:");
        Serial.println(mode);
        xSemaphoreGive(mtxSerial);
      }else if(received.type == EVENT_DISPLAY_BUTTON){
        xSemaphoreTake(mtxSerial, portMAX_DELAY);
        Serial.print("[CONTROL] Botão 2 foi pressionado. Atualização especial foi solicitada.\n");
        xSemaphoreGive(mtxSerial);
        xSemaphoreGive(semDisplaySpecial);
      }
    }
  }
}

void TaskLed(void *pv) {
  while (true) {
    xSemaphoreTake(mtxMode, portMAX_DELAY);
    int mode = systemMode;
    xSemaphoreGive(mtxMode);

    int delayLed;

    if(mode == 0){
      delayLed = 1000;
    }else{
      delayLed = 250;
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelay(pdMS_TO_TICKS(delayLed));
  }
}

void TaskStatus(void *pv) {
  while (true) {
    xSemaphoreTake(mtxMode, portMAX_DELAY);
    int mode = systemMode;
    xSemaphoreGive(mtxMode);

    xSemaphoreTake(mtxDisplay, portMAX_DELAY);
    Serial.println("---DISPLAY---");
    Serial.print("Modo: ");
    Serial.printf("%s\n", mode? "Rapido": "Lento");
    xSemaphoreGive(mtxDisplay);

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void TaskDisplaySpecial(void *pv) {
  while (true) {
    if(xSemaphoreTake(semDisplaySpecial, portMAX_DELAY) == pdTRUE){
      xSemaphoreTake(mtxDisplay, portMAX_DELAY);
      Serial.println("---DISPLAY---");
      Serial.println("Atualizacao especial");
      xSemaphoreGive(mtxDisplay);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_MODE_PIN, INPUT_PULLUP);
  pinMode(BTN_DISPLAY_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  buttonQueue = xQueueCreate(10, sizeof(ButtonEvent));
  semDisplaySpecial = xSemaphoreCreateBinary();

  mtxDisplay = xSemaphoreCreateMutex();
  mtxSerial = xSemaphoreCreateMutex();
  mtxMode = xSemaphoreCreateMutex();

  if (!buttonQueue || !semDisplaySpecial || !mtxDisplay || !mtxSerial || !mtxMode) {
    Serial.println("Erro ao criar fila, semaforo ou mutex.");
    while (true) {
      delay(1000);
    }
  }

  xTaskCreate(TaskButtons, "Buttons", 2048, NULL, 3, NULL);
  xTaskCreate(TaskControl, "Control", 2048, NULL, 3, NULL);
  xTaskCreate(TaskLed, "Led", 2048, NULL, 2, NULL);
  xTaskCreate(TaskDisplaySpecial, "DisplaySpecial", 2048, NULL, 2, NULL);
  xTaskCreate(TaskStatus, "Status", 2048, NULL, 1, NULL);
}

void loop() {
}

// https://wokwi.com/projects/464126685764028417
