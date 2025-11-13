#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_websocket_client.h"

#define WS_SERVER_URI "ws://192.168.0.100:8080"
#define DEVICE_ID "robot1"

static const char *TAG = "robot_ws";

/* --- Stub functions --- */
void move_forward() { ESP_LOGI(TAG, "[stub] move_forward()"); }
void move_backward() { ESP_LOGI(TAG, "[stub] move_backward()"); }
void turn_left() { ESP_LOGI(TAG, "[stub] turn_left()"); }
void turn_right() { ESP_LOGI(TAG, "[stub] turn_right()"); }
void stop_motion() { ESP_LOGI(TAG, "[stub] stop_motion()"); }

/* --- WS event handler --- */
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;

    switch (event_id)
    {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Websocket connected");
            esp_websocket_client_send_text(client, "register:robot:" DEVICE_ID,
                                           strlen("register:robot:" DEVICE_ID), portMAX_DELAY);
            break;

        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Websocket disconnected");
            break;

        case WEBSOCKET_EVENT_DATA:
            if (data->data_len > 0)
            {
                char buf[64]; // buffer fixo
                size_t len = data->data_len < sizeof(buf) - 1 ? data->data_len : sizeof(buf) - 1;
                memcpy(buf, data->data_ptr, len);
                buf[len] = 0;

                ESP_LOGI(TAG, "Received: %s", buf);

                if (strcasecmp(buf, "FORWARD") == 0)
                    move_forward();
                else if (strcasecmp(buf, "BACK") == 0 || strcasecmp(buf, "BACKWARD") == 0)
                    move_backward();
                else if (strcasecmp(buf, "LEFT") == 0)
                    turn_left();
                else if (strcasecmp(buf, "RIGHT") == 0)
                    turn_right();
                else if (strcasecmp(buf, "STOP") == 0)
                    stop_motion();
                else
                    ESP_LOGI(TAG, "Unknown command: %s", buf);
            }
            break;

        default:
            break;
    }
}

/* --- app_main --- */
void app_main(void)
{
    // NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_websocket_client_config_t cfg = {
        .uri = WS_SERVER_URI,
        .reconnect_timeout_ms = 5000, // reconecta automaticamente
    };

    esp_websocket_client_handle_t client = esp_websocket_client_init(&cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, client);
    esp_websocket_client_start(client);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
