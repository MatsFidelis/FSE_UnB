#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

typedef struct {
    uint8_t addr;
    uint8_t code;
    uint8_t subcode;
    uint8_t offset;
    uint8_t *data;
    uint8_t *matr;
} mb_package;

int fill_buffer(uint8_t *tx_buffer, mb_package pkg);
void print_buffer(const uint8_t *buffer, int len);
short CRC16(short crc, char data);
short get_crc(unsigned char *commands, int size);

#endif