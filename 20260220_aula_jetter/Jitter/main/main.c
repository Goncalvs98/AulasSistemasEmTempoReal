#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LED_PIN 2

static bool led_state = false;
static int64_t last_toggle_time = 0;
static int64_t last_print_time = 0;

void blink_led(){
    int64_t now = esp_timer_get_time();
    int64_t diff = now - last_toggle_time;

    if (diff >= 500000) {
        led_state = !led_state;
        gpio_set_level(LED_PIN, led_state);
        last_toggle_time = now;

        printf("[Main] Periodo = %lld us (Jitter = %lld us)\n", diff, (diff - 500000));
    }
}

void process_anything() {
    int64_t temp0 = esp_timer_get_time();
    for(volatile long i = 0; i < 500000; i++){}
    int64_t tempo_gasto = esp_timer_get_time() - temp0;

    if(esp_timer_get_time() - last_print_time >= 2000000){
        printf("[Process] Periodo = %lld us\n", tempo_gasto);
        last_print_time = esp_timer_get_time();
    }
}

void app_main(void) {
    printf("Inicio do processo \n");

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while(1){
        blink_led();
        process_anything();

        vTaskDelay(1);
    }
}

// Definindo arquitetura
// idf.py set-target esp32

// Compilando e gravando
// idf.py -p COM5 build flash monitor

// Saída:
// Inicio do processo
// [Main] Periodo = 532309 us (Jitter = 32309 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Process] Periodo = 65673 us
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Process] Periodo = 65673 us
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Process] Periodo = 65673 us
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Main] Periodo = 560000 us (Jitter = 60000 us)
// [Process] Periodo = 65673 us
// [Main] Periodo = 560000 us (Jitter = 60000 us)