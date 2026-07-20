#pragma once
#include <stdint.h>
#include <sys/types.h>

typedef enum {
    ESP32,
    IPHONE
} Message_type;

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
