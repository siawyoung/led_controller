/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/ledc.h" // Include the LEDC driver

// Define PWM properties
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FREQUENCY          (500) // 500Hz

// Define LEDC channels and their corresponding GPIOs
#define LEDC_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_OUTPUT_IO_0        (7) // GPIO 7

#define LEDC_CHANNEL_1          LEDC_CHANNEL_1
#define LEDC_OUTPUT_IO_1        (8) // GPIO 8

#define LEDC_CHANNEL_2          LEDC_CHANNEL_2
#define LEDC_OUTPUT_IO_2        (9) // GPIO 9

void app_main(void)
{
    printf("Hello world!\n");

    // Initialize LEDC timer for PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    // Set configuration of timer0 for low speed channels
    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
        printf("LEDC Timer configuration failed!\n");
        return;
    }

    // Prepare individual configuration for each channel

    // Configuration for GPIO 7 (Channel 0)
    ledc_channel_config_t ledc_channel_0 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_0,
        .duty           = 0, // Set initial duty to 0%
        .hpoint         = 0
    };
    // Set LEDC channel 0 configuration
    if (ledc_channel_config(&ledc_channel_0) != ESP_OK) {
        printf("LEDC Channel 0 configuration failed!\n");
        return;
    }

    // Configuration for GPIO 8 (Channel 1)
    ledc_channel_config_t ledc_channel_1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_1,
        .duty           = 0, // Set initial duty to 0%
        .hpoint         = 0
    };
    // Set LEDC channel 1 configuration
    if (ledc_channel_config(&ledc_channel_1) != ESP_OK) {
        printf("LEDC Channel 1 configuration failed!\n");
        return;
    }

    // Configuration for GPIO 9 (Channel 2)
    ledc_channel_config_t ledc_channel_2 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_2,
        .duty           = 0, // Set initial duty to 0%
        .hpoint         = 0
    };
    // Set LEDC channel 2 configuration
    if (ledc_channel_config(&ledc_channel_2) != ESP_OK) {
        printf("LEDC Channel 2 configuration failed!\n");
        return;
    }

    // Set duty cycle to 10% for all channels
    uint32_t duty = (1 << LEDC_DUTY_RES) / 10; // 10% duty cycle

    // Update duty for Channel 0 (GPIO 7)
    if (ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty) != ESP_OK) {
        printf("LEDC Set duty for Channel 0 failed!\n");
        return;
    }
    if (ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0) != ESP_OK) {
        printf("LEDC Update duty for Channel 0 failed!\n");
        return;
    }

    // Update duty for Channel 1 (GPIO 8)
    if (ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty) != ESP_OK) {
        printf("LEDC Set duty for Channel 1 failed!\n");
        return;
    }
    if (ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1) != ESP_OK) {
        printf("LEDC Update duty for Channel 1 failed!\n");
        return;
    }

    // Update duty for Channel 2 (GPIO 9)
    if (ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty) != ESP_OK) {
        printf("LEDC Set duty for Channel 2 failed!\n");
        return;
    }
    if (ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2) != ESP_OK) {
        printf("LEDC Update duty for Channel 2 failed!\n");
        return;
    }

    printf("PWM signals are being output on GPIO %d, GPIO %d, and GPIO %d with %d Hz frequency and 10%% duty cycle.\n",
           LEDC_OUTPUT_IO_0, LEDC_OUTPUT_IO_1, LEDC_OUTPUT_IO_2, LEDC_FREQUENCY);

    // Optionally, you can adjust the duty cycle in a loop for all channels
    // while (1) {
    //     for (int duty = 0; duty <= (1 << LEDC_DUTY_RES); duty += 100) {
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty);
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty);
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2);
    //         vTaskDelay(10 / portTICK_PERIOD_MS);
    //     }
    //     for (int duty = (1 << LEDC_DUTY_RES); duty >= 0; duty -= 100) {
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty);
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty);
    //         ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
    //         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2);
    //         vTaskDelay(10 / portTICK_PERIOD_MS);
    //     }
    // }

    // Keep the task alive indefinitely
    // while (1) {
    //     vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    // }
}
