// Alterações para deixar o sistema mais agressivo.
// RECV_TIMEOUT_MS: 1000ms -> 750ms
// Adicionando as constantes TRIES_TO_RESET_QUEUE e TRIES_TO_ALARM para melhor manutenção.
// TRIES_TO_RESET_QUEUE: 3 -> 2
// TRIES_TO_ALARM: 5 -> 4

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#define BTN_FAULT_PIN  4
#define LED_OK_PIN     2
#define LED_ERR_PIN    19

#define TRIES_TO_RESET_QUEUE 2
#define TRIES_TO_ALARM 4

#define SEND_TIMEOUT_MS   50
#define RECV_TIMEOUT_MS   750
#define MUTEX_TIMEOUT_MS  100
#define ALARM_TIMEOUT_MS  5000

QueueHandle_t sampleQueue;

SemaphoreHandle_t semAlarm;

SemaphoreHandle_t mtxCounter;

int processed = 0;

bool faultInjected = false;

void TaskSensor(void *pv) {
  int value;

  while (true) {
    if (!faultInjected) {
      value = random(0, 100);

      if (xQueueSend(sampleQueue, &value, pdMS_TO_TICKS(SEND_TIMEOUT_MS)) != pdTRUE) {
        Serial.println("[SENSOR] fila cheia, amostra descartada");
      }
    } else {
      Serial.println("[SENSOR] falha injetada, producao parada");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void TaskProcess(void *pv) {
  int received;
  int failStreak = 0;

  while (true) {
    if (xQueueReceive(sampleQueue, &received, pdMS_TO_TICKS(RECV_TIMEOUT_MS)) == pdTRUE) {

      if (failStreak > 0) {
        Serial.println("[PROC] fluxo restabelecido");
        failStreak = 0;
      }

      Serial.print("[PROC] amostra recebida: ");
      Serial.println(received);

      xSemaphoreTake(mtxCounter, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS));
      processed++;
      xSemaphoreGive(mtxCounter);

      digitalWrite(LED_OK_PIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(100));
      digitalWrite(LED_OK_PIN, LOW);

    } else {
      failStreak++;

      Serial.print("[PROC] timeout sem dados: ");
      Serial.println(failStreak);

      if (failStreak == TRIES_TO_RESET_QUEUE) {
        Serial.println("[PROC] recuperacao: esvaziando a fila");
        xQueueReset(sampleQueue);
      }

      if (failStreak >= TRIES_TO_ALARM) {
        Serial.println("[PROC] falha persistente, acionando o alarme");
        xSemaphoreGive(semAlarm);
        failStreak = 0;
      }
    }
  }
}

void TaskAlarm(void *pv) {
  while (true) {
    if (xSemaphoreTake(semAlarm, pdMS_TO_TICKS(ALARM_TIMEOUT_MS)) == pdTRUE) {

      xSemaphoreTake(mtxCounter, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS));
      int total = processed;
      xSemaphoreGive(mtxCounter);

      Serial.print("[ALARME] cadeia interrompida. Amostras processadas: ");
      Serial.println(total);

      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_ERR_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(150));
        digitalWrite(LED_ERR_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(150));
      }

    } else {
      Serial.println("[ALARME] sem eventos, sistema saudavel");
    }
  }
}

void TaskButton(void *pv) {
  int lastBtn = HIGH;

  while (true) {
    int btn = digitalRead(BTN_FAULT_PIN);

    if (lastBtn == HIGH && btn == LOW) {
      faultInjected = !faultInjected;

      Serial.print("[BOTAO] falha injetada: ");
      Serial.println(faultInjected);

      vTaskDelay(pdMS_TO_TICKS(200));
    }

    lastBtn = btn;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_FAULT_PIN, INPUT_PULLUP);
  pinMode(LED_OK_PIN, OUTPUT);
  pinMode(LED_ERR_PIN, OUTPUT);

  sampleQueue = xQueueCreate(5, sizeof(int));
  semAlarm = xSemaphoreCreateBinary();
  mtxCounter = xSemaphoreCreateMutex();

  if (!sampleQueue || !semAlarm || !mtxCounter) {
    Serial.println("Erro ao criar fila, semaforo ou mutex.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("Robustez 1: erros e timeout. Botao no GPIO4 injeta a falha.");

  xTaskCreate(TaskSensor, "Sensor", 2048, NULL, 3, NULL);
  xTaskCreate(TaskProcess, "Process", 2048, NULL, 3, NULL);
  xTaskCreate(TaskAlarm, "Alarm", 2048, NULL, 2, NULL);
  xTaskCreate(TaskButton, "Button", 2048, NULL, 2, NULL);
}

void loop() {
}
