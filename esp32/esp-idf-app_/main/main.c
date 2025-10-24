// main/main.c
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

// Configure this to the host IP where server runs (your PC)
#define WS_SERVER_URI "ws://192.168.0.100:8080"

static const char *TAG = "robot_ws";

/* --- stubs to be implemented on the real hardware --- */
void move_forward() { ESP_LOGI(TAG, "[stub] move_forward()"); }
void move_backward() { ESP_LOGI(TAG, "[stub] move_backward()"); }
void turn_left() { ESP_LOGI(TAG, "[stub] turn_left()"); }
void turn_right() { ESP_LOGI(TAG, "[stub] turn_right()"); }
void stop_motion() { ESP_LOGI(TAG, "[stub] stop_motion()"); }

/* --- WS event handler --- */
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
  switch (event_id)
  {
  case WEBSOCKET_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Websocket connected");
    // register as robot to server
    esp_websocket_client_send_text((esp_websocket_client_handle_t)handler_args, "register:robot:robot1", strlen("register:robot:robot1"), portMAX_DELAY);
    break;
  case WEBSOCKET_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "Websocket disconnected");
    break;
  case WEBSOCKET_EVENT_DATA:
    if (data->data_len > 0)
    {
      char *buf = malloc(data->data_len + 1);
      memcpy(buf, data->data_ptr, data->data_len);
      buf[data->data_len] = 0;
      ESP_LOGI(TAG, "Received: %s", buf);

      // parse simple action strings: e.g. "FORWARD" / "LEFT" / "STOP"
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

      free(buf);
    }
    break;
  default:
    break;
  }
}

/* --- app_main: initialize NVS, netif and start websocket client --- */
void app_main(void)
{
  // Init NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    nvs_flash_erase();
    nvs_flash_init();
  }

  // Initialize TCP/IP network interface (required for sockets)
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // NOTE: In QEMU environment, Wi-Fi won't be used; ESP-IDF's lwIP will use QEMU's SLIRP network.
  // No Wi-Fi initialization here; we rely on the host-networking the emulator provides.

  esp_websocket_client_config_t cfg = {
      .uri = WS_SERVER_URI,
  };

  esp_websocket_client_handle_t client = esp_websocket_client_init(&cfg);

  // Register event handler
  esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, client);

  // Start client
  esp_websocket_client_start(client);

  // Keep running
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
