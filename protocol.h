#pragma once
#include <stdint.h>
#include <sys/types.h>

typedef enum {
    ESP32,
    IPHONE
} Message_type;

typedef struct {
    uint8_t type;
    uint16_t length;
} Header;

void header_pack(const Header *h, uint8_t *buf);
void header_unpack(const uint8_t *buf, Header *h);
ssize_t read_header(int fd, Header *h);
