#include "protocol.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Структура с заголовком имеет размер 3 байта,
* но компилятор может добавить +1 байт, чтобы адрес
* начинался с четного адреса в памяти. По сети может уйти лишний байт.
* Чтобы решить это, заполняю буфер вручную
*/

#define HEADER_SIZE 3

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

ssize_t read_header(int fd, Header *h) {
    uint8_t buf[HEADER_SIZE];
    ssize_t n;
    if ((n = recv_all(fd, buf, HEADER_SIZE)) < 1)
        return n;
    header_unpack(buf, h); 
    return 1;
}

ssize_t recv_all(int fd, uint8_t *buf, uint16_t length) {
    ssize_t n = 0;
    size_t total_read = 0;
    while (total_read < length) {
        if ((n = recv(fd, buf + total_read, length - total_read, 0)) < 1)
            return n;
        total_read += n;
    }
    return total_read;
}

void esp32_data_pack(const ESP32_data *data, uint8_t *buf) {

}
