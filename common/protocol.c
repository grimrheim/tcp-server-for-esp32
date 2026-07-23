#include "protocol.h"
#include <netinet/in.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

/*
 * Структура с заголовком имеет размер 3 байта,
 * но компилятор может добавить +1 байт, чтобы адрес
 * начинался с четного адреса в памяти. По сети может уйти лишний байт.
 * Чтобы решить это, заполняю буфер вручную
 */

// байты для отправки
void header_pack(const Header *h, uint8_t *buf) {
    buf[0] = h->type;
    uint16_t len_be = htons(h->length);
    memcpy(buf + 1, &len_be, 2);
}

// байты, пришедшие в сеть
void header_unpack(const uint8_t *buf, Header *h) {
    h->type = buf[0];
    uint16_t len_be;
    memcpy(&len_be, buf + 1, 2);
    h->length = ntohs(len_be);
}

size_t ack_pack(uint8_t *out_buf, uint8_t status) {
    Header hdr = {
        .type = ACK_TYPE,
        .length = sizeof(status)
    };
    header_pack(&hdr, out_buf);
    out_buf[HEADER_SIZE] = status;
    return HEADER_SIZE + sizeof(status);
}

size_t cmd_pack(uint8_t *out_buf, uint8_t status) {
    Header hdr = {
        .type = CMD_REMOTE,
        .length = sizeof(status)
    };
    header_pack(&hdr, out_buf);
    out_buf[HEADER_SIZE] = status;
    return HEADER_SIZE + sizeof(status);
}

ssize_t read_header(int fd, Header *h) {
    uint8_t buf[HEADER_SIZE];
    ssize_t n;
    if ((n = recv_all(fd, buf, HEADER_SIZE)) < 1)
        return n;
    header_unpack(buf, h); 
    return n;
}

ssize_t recv_all(int fd, uint8_t *buf, uint16_t length) {
    ssize_t n = 0;
    size_t total_read = 0;
    while (total_read < length) {
        if ((n = recv(fd, buf + total_read, length - total_read, 0)) < 1) {
            if (n < 0) {
                printf("recv errno=%d\n", errno);
                perror("recv");
                return -1;
            }
            if (n == 0) {
                printf("peer closed conection\n");
                return 0;
            }
        }
        total_read += n;        
    }
    return total_read;
}

void read_esp_data(uint8_t *payload) {
    /*
    * FF:FF:FF:FF:FF:FF + \0 = 18 байт
    * 1 байт бинарных данных -> 2 символа hex
    * плюс 5 байт на ':'
    */
    char mac_str[18];
    /*
    * 255.255.255.255 + \0 = 16 bytes
    * 1 байт бинарных данных -> 3 символа decimal
    * плюс 3 байта на '.'
    */
    char ip_str[16];

    snprintf(mac_str, sizeof(mac_str),
            "%02x:%02x:%02x:%02x:%02x:%02x",
            payload[0], payload[1], payload[2], payload[3], payload[4], payload[5]);
    snprintf(ip_str, sizeof(ip_str),
            "%d.%d.%d.%d",
            payload[6], payload[7], payload[8], payload[9]);

    printf("MAC: %s, IP: %s\n", mac_str, ip_str);
}
