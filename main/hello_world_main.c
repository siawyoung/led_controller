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
#define LEDC_OUTPUT_IO          (7) // GPIO 7
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FREQUENCY          (500) // 500 Hz

void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT/" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE/" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? "802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // Initialize LEDC for PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    // Set configuration of timer0 for high speed channels
    if (ledc_timer_config(&ledc_timer) != ESP_OK) {
        printf("LEDC Timer configuration failed!\n");
        return;
    }

    // Prepare individual configuration for each channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set initial duty to 0%
        .hpoint         = 0
    };
    // Set LEDC channel configuration
    if (ledc_channel_config(&ledc_channel) != ESP_OK) {
        printf("LEDC Channel configuration failed!\n");
        return;
    }

    // Set duty cycle to 50%
    uint32_t duty = (1 << LEDC_DUTY_RES) / 2;
    if (ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty) != ESP_OK) {
        printf("LEDC Set duty failed!\n");
        return;
    }
    if (ledc_update_duty(LEDC_MODE, LEDC_CHANNEL) != ESP_OK) {
        printf("LEDC Update duty failed!\n");
        return;
    }
    printf("PWM signal is being output on GPIO %d with %d Hz frequency and 50%% duty cycle.\n",
           LEDC_OUTPUT_IO, LEDC_FREQUENCY);

    // Optionally, you can adjust the duty cycle in a loop
    /*
    while (1) {
        for (int duty = 0; duty <= (1 << LEDC_DUTY_RES); duty += 100) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        for (int duty = (1 << LEDC_DUTY_RES); duty >= 0; duty -= 100) {
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    */

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
