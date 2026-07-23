#pragma once
#include <stdint.h>
#include <sys/types.h>

#define HEADER_SIZE 3
#define MAX_PAYLOAD 65535

// Типы клиентов
#define ESP_TYPE        0x01
// Типы ACK сообщения
#define ACK_TYPE        0x00
#define ACK_SUCCESS     0x00
#define ACK_ERROR       0x01
// Типы команд
#define CMD_REMOTE      0x10
#define CMD_LED_BLINK   0x11
#define CMD_LED_OFF     0x12
#define CMD_LED_ON      0x13

/*
 * type - тип устройства клиента,
 * length - длинна сообщения
 * __attribute__ - чтобы исключить ошибку между GCC/Clang и Xtensa
 */
typedef struct __attribute__((packed)){
    uint8_t type;
    uint16_t length;
} Header;

typedef struct {
    uint8_t mac[6];
    uint8_t ip[4];
} ESP32_data;

void header_pack(const Header *h, uint8_t *buf);
void header_unpack(const uint8_t *buf, Header *h);
ssize_t read_header(int fd, Header *h);
ssize_t recv_all(int fd, uint8_t *buf, uint16_t length);
void read_esp_data(uint8_t *payload);
size_t ack_pack(uint8_t *out_buf, uint8_t status);
size_t cmd_pack(uint8_t *out_buf, uint8_t status);
