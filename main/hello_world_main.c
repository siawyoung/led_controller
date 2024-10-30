#include <string.h>
#include <stdio.h>
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
#define LEDC_OUTPUT_IO_0        7

static const char *TAG = "PWM_WebServer";

// HTML content for the web page
const char* html_page = 
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>PWM Controller</title></head>"
    "<body>"
    "<h1>Adjust PWM Duty Cycle</h1>"
    "<input type='range' min='0' max='8191' value='819' id='dutySlider' oninput='updateDuty(this.value)'/>"
    "<p>Duty Cycle: <span id='dutyValue'>10%</span></p>"
    "<script>"
    "function updateDuty(val) {"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('GET', '/set?duty=' + val, true);"
    "  xhr.send();"
    "  document.getElementById('dutyValue').innerText = (val / 8191 * 100).toFixed(2) + '%';"
    "}"
    "</script>"
    "</body>"
    "</html>";

// Function to initialize LEDC for PWM
void init_pwm() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_0,
        .duty           = 819, // Initial duty cycle (10%)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

// Handler to serve the HTML page
esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

// Handler to set the duty cycle
esp_err_t set_duty_handler(httpd_req_t *req) {
    char duty_str[10];
    int ret = httpd_req_get_url_query_str(req, duty_str, sizeof(duty_str));
    if (ret == ESP_OK) {
        char duty_val[10];
        if (httpd_query_key_value(duty_str, "duty", duty_val, sizeof(duty_val)) == ESP_OK) {
            int duty = atoi(duty_val);
            if (duty >= 0 && duty <= 8191) { // 13-bit resolution
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
                ESP_LOGI(TAG, "Duty cycle set to %d (%.2f%%)", duty, (duty / 8191.0) * 100);
            }
        }
    }
    httpd_resp_send(req, "OK", 2);
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
