#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"

#define BTN_SOFT_PIN   4
#define BTN_HARD_PIN   5
#define LED_A_PIN      2
#define LED_B_PIN      18

#define WDT_TIMEOUT_S       5
#define HEARTBEAT_LIMIT_MS  2000
#define MAX_RECREATIONS     3

TaskHandle_t hWorkerB = NULL;

volatile unsigned long heartbeatB = 0;

bool softFault = false;
bool hardFault = false;

int recreations = 0;

RTC_DATA_ATTR int bootCount = 0;

void TaskWorkerA(void *pv) {
  esp_task_wdt_add(NULL);

  while (true) {
    if(hardFault){
      Serial.println("[WORKER A] tarefa está travada. WDT não é resetado");
      while(true); // Travamento
    }
    digitalWrite(LED_A_PIN, !digitalRead(LED_A_PIN));

    esp_task_wdt_reset();

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void TaskWorkerB(void *pv) {
  heartbeatB = millis();

  while (true) {
    digitalWrite(LED_B_PIN, !digitalRead(LED_B_PIN));

    if(!softFault){
      heartbeatB = millis();
    }else{
      Serial.println("[WORKER B] não tem sinal de vida");
    }

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void TaskSupervisor(void *pv) {
  esp_task_wdt_add(NULL);

  while (true) {
    unsigned long age = millis() - heartbeatB;

    if(age > HEARTBEAT_LIMIT_MS){
      Serial.print("[SUPERVISOR] tarefa worker B parada há ms: ");
      Serial.println(age);

      if (recreations >= MAX_RECREATIONS){
        Serial.println("[SUPERVISOR] Tentativa de recuperação esgotada, reiniciando o sistema...");
        esp_restart();
      }

      vTaskDelete(hWorkerB);

      softFault = false;
      recreations++;
      heartbeatB = millis();

      
      xTaskCreate(TaskWorkerB, "WorkerB", 2048, NULL, 2, &hWorkerB);
      Serial.print("[SUPERVISOR] tarefa recriada, Tentativa: ");
      Serial.println(recreations);
    }

    esp_task_wdt_reset();

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void TaskButtons(void *pv) {
  int lastSoft = HIGH;
  int lastHard = HIGH;

  while (true) {
    int soft = digitalRead(BTN_SOFT_PIN);
    int hard = digitalRead(BTN_HARD_PIN);

    if (lastSoft == HIGH && soft == LOW) {
      softFault = true;
      Serial.println("[BOTAO] falha recuperavel na worker B");
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (lastHard == HIGH && hard == LOW) {
      hardFault = true;
      Serial.println("[BOTAO] falha fatal na worker A, aguarde o watchdog");
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    lastSoft = soft;
    lastHard = hard;
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setupWatchdog() {
#if ESP_IDF_VERSION_MAJOR >= 5
  esp_task_wdt_config_t config;
  config.timeout_ms = WDT_TIMEOUT_S * 1000;
  config.idle_core_mask = 0;
  config.trigger_panic = true;

  if (esp_task_wdt_init(&config) == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_reconfigure(&config);
  }
#else
  esp_task_wdt_init(WDT_TIMEOUT_S, true);
#endif
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_SOFT_PIN, INPUT_PULLUP);
  pinMode(BTN_HARD_PIN, INPUT_PULLUP);
  pinMode(LED_A_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);

  bootCount++;

  Serial.println("Robustez 2: watchdog e recuperacao");
  Serial.print("Boot numero: ");
  Serial.println(bootCount);

  if (esp_reset_reason() == ESP_RST_TASK_WDT) {
    Serial.println("O reinicio anterior foi provocado pelo TASK WATCHDOG");
  }

  Serial.println("GPIO4 = falha recuperavel | GPIO5 = falha fatal");

  setupWatchdog();

  heartbeatB = millis();

  xTaskCreate(TaskSupervisor, "Supervisor", 2048, NULL, 4, NULL);
  xTaskCreate(TaskWorkerA, "WorkerA", 2048, NULL, 3, NULL);
  xTaskCreate(TaskWorkerB, "WorkerB", 2048, NULL, 2, &hWorkerB);
  xTaskCreate(TaskButtons, "Buttons", 2048, NULL, 2, NULL);
}

void loop() {
}

// https://wokwi.com/projects/473006402882741249
