#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/modbus.h"

int open_uart(const char *device);
void send_uart(int fd, const uint8_t *buffer, int len);
int receive_uart(int fd, uint8_t *buffer, int max_len);
void close_uart(int fd);

#define UART_DEVICE "/dev/serial0"
#define MAX_BUFFER 256
static uint8_t MATR[4] = {0, 0, 6, 6};

void verificar_botoes() {
    uint8_t buffer[MAX_BUFFER];
    uint8_t dados[2] = {0x00, 0x05}; // registrador inicial 0x00, quantidade 5

    mb_package pkg = {
        .addr = 0x01,
        .code = 0x03,
        .offset = 2,
        .data = dados,
        .matr = MATR
    };

    int len = fill_buffer(buffer, pkg);
    int uart_fd = open_uart(UART_DEVICE);
    if (uart_fd < 0) return;

    send_uart(uart_fd, buffer, len);
    usleep(50000);

    uint8_t response[MAX_BUFFER];
    int rcv = receive_uart(uart_fd, response, MAX_BUFFER);
    close_uart(uart_fd);

    if (rcv < 9) {
        printf("[!] Resposta muito curta (rcv = %d)\n", rcv);
        return;
    }

    short crc_calc = get_crc(response, rcv - 2);
    short crc_resp = *(short*)&response[rcv - 2];
    if (crc_calc != crc_resp) {
        printf("[!] CRC inválido\n");
        return;
    }

    // Correção da ordem real dos registradores
    uint8_t reg_x     = response[2]; // Esq/Dir
    uint8_t reg_y     = response[3]; //Cima/Baixo
    uint8_t reg_pos   = response[4]; // P1–P4
    uint8_t reg_prog  = response[5]; // Programar
    uint8_t reg_calib = response[6]; // Calibrar

    printf("Botões: ");
    printf("[Cima: %d] ",   (reg_y & 0x01) ? 1 : 0);
    printf("[Baixo: %d] ",  (reg_y & 0x02) ? 1 : 0);
    printf("[Esq: %d] ",    (reg_x & 0x01) ? 1 : 0);
    printf("[Dir: %d] ",    (reg_x & 0x02) ? 1 : 0);
    printf("[P1: %d] ",     (reg_pos & 0x01) ? 1 : 0);
    printf("[P2: %d] ",     (reg_pos & 0x02) ? 1 : 0);
    printf("[P3: %d] ",     (reg_pos & 0x04) ? 1 : 0);
    printf("[P4: %d] ",     (reg_pos & 0x08) ? 1 : 0);
    printf("[Prog: %d] ",   reg_prog ? 1 : 0);
    printf("[Calib: %d]\n", reg_calib ? 1 : 0);
}

int main() {
    while (1) {
        verificar_botoes();
        usleep(50000);
    }
    return 0;
}