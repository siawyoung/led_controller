#include <string.h>
#include <stdio.h>
#include <stdlib.h> // For atoi
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY          500
#define LEDC_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_CHANNEL_1          LEDC_CHANNEL_1
#define LEDC_CHANNEL_2          LEDC_CHANNEL_2
#define LEDC_OUTPUT_IO_0        7
#define LEDC_OUTPUT_IO_1        8
#define LEDC_OUTPUT_IO_2        9

static const char *TAG = "PWM_WebServer";

// HTML content for the web page with three text inputs
const char* html_page = 
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>PWM Controller</title></head>"
    "<body>"
    "<h1>Adjust PWM Duty Cycles</h1>"
    "<div>"
    "  <label for='duty0'>GPIO 7 Duty Cycle (0-8191): </label>"
    "  <input type='number' id='duty0' name='duty0' min='0' max='8191' value='819' oninput='updateDuty(0, this.value)'/>"
    "</div>"
    "<div>"
    "  <label for='duty1'>GPIO 8 Duty Cycle (0-8191): </label>"
    "  <input type='number' id='duty1' name='duty1' min='0' max='8191' value='819' oninput='updateDuty(1, this.value)'/>"
    "</div>"
    "<div>"
    "  <label for='duty2'>GPIO 9 Duty Cycle (0-8191): </label>"
    "  <input type='number' id='duty2' name='duty2' min='0' max='8191' value='819' oninput='updateDuty(2, this.value)'/>"
    "</div>"
    "<p>Duty Cycles Updated</p>"
    "<script>"
    "function updateDuty(channel, val) {"
    "  if(val === '') return;"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('GET', '/set?channel=' + channel + '&duty=' + val, true);"
    "  xhr.send();"
    "}"
    "</script>"
    "</body>"
    "</html>";

// Function to initialize LEDC for PWM on multiple channels
void init_pwm() {
    // Configure LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channels
    ledc_channel_config_t ledc_channel_0 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_0,
        .duty           = 819, // Initial duty cycle (10%)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_0);

    ledc_channel_config_t ledc_channel_1 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_1,
        .duty           = 819, // Initial duty cycle (10%)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_1);

    ledc_channel_config_t ledc_channel_2 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_2,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_2,
        .duty           = 819, // Initial duty cycle (10%)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel_2);
}

// Handler to serve the HTML page
esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

// Handler to set the duty cycle for a specific channel
esp_err_t set_duty_handler(httpd_req_t *req) {
    char query_str[100];
    if (httpd_req_get_url_query_str(req, query_str, sizeof(query_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No query string found");
        return ESP_FAIL;
    }

    char channel_str[10];
    char duty_val_str[10];
    if (httpd_query_key_value(query_str, "channel", channel_str, sizeof(channel_str)) != ESP_OK ||
        httpd_query_key_value(query_str, "duty", duty_val_str, sizeof(duty_val_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing channel or duty parameter");
        return ESP_FAIL;
    }

    int channel = atoi(channel_str);
    int duty = atoi(duty_val_str);

    if (channel < 0 || channel > 2) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid channel");
        return ESP_FAIL;
    }

    if (duty < 0 || duty > 8191) { // 13-bit resolution
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Duty must be between 0 and 8191");
        return ESP_FAIL;
    }

    // Set duty cycle based on channel
    switch(channel) {
        case 0:
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
            ESP_LOGI(TAG, "GPIO 7 Duty cycle set to %d (%.2f%%)", duty, (duty / 8191.0) * 100);
            break;
        case 1:
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
            ESP_LOGI(TAG, "GPIO 8 Duty cycle set to %d (%.2f%%)", duty, (duty / 8191.0) * 100);
            break;
        case 2:
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2);
            ESP_LOGI(TAG, "GPIO 9 Duty cycle set to %d (%.2f%%)", duty, (duty / 8191.0) * 100);
            break;
        default:
            // This should never happen due to earlier checks
            break;
    }

    httpd_resp_send(req, "OK", strlen("OK"));
    return ESP_OK;
}

// Function to start the web server
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t set_uri = {
            .uri       = "/set",
            .method    = HTTP_GET,
            .handler   = set_duty_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &set_uri);
    }
    return server;
}

// Function to initialize Wi-Fi as Access Point
void init_wifi() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid       = "ESP32_PWM_AP",
            .ssid_len   = strlen("ESP32_PWM_AP"),
            .password   = "12345678",
            .max_connection = 4,
            .authmode   = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized. AP SSID: %s, Password: %s", wifi_config.ap.ssid, wifi_config.ap.password);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting PWM Web Server Example");

    init_pwm();
    init_wifi();
    start_webserver();

    ESP_LOGI(TAG, "Web server started. Connect to AP and navigate to http://192.168.4.1/");
}
