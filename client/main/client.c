#include <machine/endian.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "protocol.h"
#include "esp_event.h"
#include "nvs_flash.h"

#define SERVER_IP "192.168.0.7"
#define SERVER_PORT 3490

static const char *TAG = "tcp_client";

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

        // Формирование TLV-пакета
        uint8_t payload = 0x42;     // Это тестовая нагрузка, пока не дописан протокол
        Header hdr = {
            .type = 0x01,
            .length = htons(sizeof(payload))
        };
        
        uint8_t frame[sizeof(hdr) + sizeof(payload)];
        memcpy(frame, &hdr, sizeof(hdr));
        memcpy(frame + sizeof(hdr), &payload, sizeof(payload));

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

        shutdown(sock, SHUT_RDWR);
        close(sock);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    tcp_client_task(void *pvParameters)
}
