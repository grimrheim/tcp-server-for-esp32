#include <machine/endian.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "esp_err.h"
#include "esp_event_base.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "nvs.h"
#include "portmacro.h"
#include "protocol.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "creds.h"
#include <errno.h>


static const char *TAG = "tcp_client";
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

void send_data(int sock) {
    // Формирование TLV-пакета
    ESP32_data payload = {};
    esp_err_t mac_err = esp_wifi_get_mac(WIFI_IF_STA, payload.mac);
    if (mac_err != ESP_OK) ESP_LOGE(TAG, "esp_wifi_get_mac failed: %s", esp_err_to_name(mac_err));

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;
    esp_err_t ip_err = esp_netif_get_ip_info(netif, &ip_info);
    if (ip_err != ESP_OK) ESP_LOGE(TAG, "esp_netif_get_ip_info failed: %s", esp_err_to_name(ip_err));
    memcpy(payload.ip, &ip_info.ip.addr, 4);


    Header hdr = {
        .type = ESP_TYPE,
        .length = sizeof(payload)
    };

    uint8_t frame[sizeof(hdr) + sizeof(payload)];
    header_pack(&hdr, frame);
    memcpy(frame + HEADER_SIZE, &payload, sizeof(payload));

    int to_write = sizeof(frame);
    int written = 0;
    while (to_write > 0) {
        int n = send(sock, frame + written, to_write, 0);
        if (n < 0) {
            ESP_LOGE(TAG, "send() failed: errno %d", errno);
            break;
        }
        written += n;
        to_write -= n;
    }
    ESP_LOGI(TAG, "sent %d bytes", written);
}

void receive_data(int sock) {
    uint8_t message_buf[HEADER_SIZE + 1];       // Header + status
    int message_len = recv_all(sock, message_buf, sizeof(message_buf));
    ESP_LOGI(TAG, "received %d bytes", message_len);
    for (size_t i = 0; i < message_len; i++) {
        ESP_LOGI(TAG, "byte[%d]=0x%02x",i, message_buf[i]);
    }
    Header hdr;
    if (message_len == sizeof(message_buf)) {
        header_unpack(message_buf, &hdr);
        uint8_t status = message_buf[HEADER_SIZE];
        if (hdr.type == ACK_TYPE) {
            if (status == ACK_SUCCESS) {
                ESP_LOGI(TAG, "server acked, success");
            }
        }
        else if (hdr.type == CMD_REMOTE) {
            if (status == CMD_LED_BLINK) {

            }
        }
    }
    else if (message_len < 0) {
        ESP_LOGE(TAG, "recv(ack) failed: errno %d", errno);
    } else {
        ESP_LOGW(TAG, "server closed connection without ack");
    }
}

void tcp_client_task(void *pvParameters) {
    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SERVER_PORT);

        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket(): failed: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "connect() failed: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        ESP_LOGI(TAG, "connected to %s:%d", SERVER_IP, SERVER_PORT);

        send_data(sock);
        receive_data(sock);

        shutdown(sock, SHUT_RDWR);
        close(sock);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Обработчик событий Wi-Fi
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "disconnected, retrying...");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished, waiting for connection...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                        pdFALSE, pdTRUE, portMAX_DELAY);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
