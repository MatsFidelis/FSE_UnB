#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../include/dashboard.h"
#include "../include/modbus.h"

#define UART_DEVICE "/dev/serial0"
static uint8_t MATR[4] = {3, 1, 8, 4}; // ajuste para sua matrícula

extern int open_uart(const char *device);
extern void send_uart(int fd, const uint8_t *buffer, int len);
extern void close_uart(int fd);


// Função genérica para enviar um float para qualquer registrador
void dashboard_write_float(uint8_t reg, float valor) {
    uint8_t buffer[256];
    uint8_t data[6];
    data[0] = reg;   // registrador
    data[1] = 0x04;  // agora: quantidade de bytes (4 para float)
    memcpy(&data[2], &valor, sizeof(float));

    mb_package pkg = {
        .addr = 0x01,
        .code = 0x06,
        .offset = 6,
        .data = data,
        .matr = MATR
    };

    int len = fill_buffer(buffer, pkg);
    //print_buffer(buffer, len); // Debug!
    int uart_fd = open_uart(UART_DEVICE);
    if (uart_fd < 0) return;

    send_uart(uart_fd, buffer, len);
    usleep(100000);
    close_uart(uart_fd);
}

void dashboard_write_estado(uint8_t estado) {
    uint8_t buffer[256];
    uint8_t data[3]; // [endereço][quantidade][dado]
    data[0] = 0x1D;      // endereço do registrador de estado
    data[1] = 0x01;      // quantidade de bytes (1 byte)
    data[2] = estado ? 1 : 0; // 0 ou 1

    mb_package pkg = {
        .addr = 0x01,
        .code = 0x06,
        .offset = 3,
        .data = data,
        .matr = MATR
    };

    int len = fill_buffer(buffer, pkg);
    //print_buffer(buffer, len); // debug
    int uart_fd = open_uart(UART_DEVICE);
    if (uart_fd < 0) return;

    send_uart(uart_fd, buffer, len);
    usleep(100000);
    close_uart(uart_fd);
}



// Wrappers para cada dado do display
void dashboard_write_vx(float vx) { dashboard_write_float(0x05, vx); }
void dashboard_write_vy(float vy) { dashboard_write_float(0x09, vy); }
void dashboard_write_x(float x)   { dashboard_write_float(0x0D, x); }
void dashboard_write_y(float y)   { dashboard_write_float(0x11, y); }
void dashboard_write_temp(float temp)   { dashboard_write_float(0x15, temp); }
void dashboard_write_press(float press) { dashboard_write_float(0x19, press); }