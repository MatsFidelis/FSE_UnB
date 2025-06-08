#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/modbus.h"
#include "../include/raiox.h"
#include "../include/botoes_fisicos.h"
#include "../include/encoder.h"
#include "../include/motores.h"
#include "../include/dashboard.h"
#include "calibracao.h"

#define UART_DEVICE "/dev/serial0"
#define MAX_BUFFER 256

static uint8_t MATR[4] = {0, 0, 6, 6};

// Protótipos
int open_uart(const char *device);
void send_uart(int fd, const uint8_t *buffer, int len);
int receive_uart(int fd, uint8_t *buffer, int max_len);
void close_uart(int fd);

// Comando 0x06 MODBUS customizado (com qtd + matrícula)
void resetar_registrador(uint8_t endereco_reg) {
    uint8_t buffer[256];

    buffer[0] = 0x01;          // Endereço da ESP32
    buffer[1] = 0x06;          // Código da função
    buffer[2] = endereco_reg;  // Endereço do registrador
    buffer[3] = 0x01;          // Quantidade (1 byte)
    buffer[4] = 0x00;          // Valor para resetar

    // Matrícula de 4 bytes
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x06;
    buffer[8] = 0x06;

    // CRC
    short crc = get_crc(buffer, 9);
    memcpy(&buffer[9], &crc, 2); // 2 bytes de CRC

    int uart_fd = open_uart(UART_DEVICE);
    if (uart_fd < 0) return;

    send_uart(uart_fd, buffer, 11); // 9 bytes + 2 de CRC
    usleep(50000);

    uint8_t response[256];
    int rcv = receive_uart(uart_fd, response, sizeof(response));
    close_uart(uart_fd);

    if (rcv < 9) {
        //printf("[!] Resposta curta no reset de registrador (rcv = %d)\n", rcv);
        return;
    }

    short crc_calc = get_crc(response, rcv - 2);
    short crc_resp = *(short*)&response[rcv - 2];
    if (crc_resp != crc_calc) {
        printf("[!] CRC inválido na resposta do reset.\n");
    }
}

void verificar_botoes() {
    uint8_t buffer[MAX_BUFFER];
    uint8_t dados[2] = {0x00, 0x05}; // Endereço inicial, Qtd

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

    uint8_t reg_x     = response[3];
    uint8_t reg_y     = response[4];
    uint8_t reg_pos   = response[5];
    uint8_t reg_prog  = response[6];
    uint8_t reg_calib = response[7];

    //printf("Botões: ");
    //printf("[Cima: %d] ",   (reg_y & 0x01) ? 1 : 0);
    //printf("[Baixo: %d] ",  (reg_y & 0x02) ? 1 : 0);
    //printf("[Esq: %d] ",    (reg_x & 0x01) ? 1 : 0);
    //printf("[Dir: %d] ",    (reg_x & 0x02) ? 1 : 0);
    //printf("[P1: %d] ",     (reg_pos & 0x01) ? 1 : 0);
    //printf("[P2: %d] ",     (reg_pos & 0x02) ? 1 : 0);
    //printf("[P3: %d] ",     (reg_pos & 0x04) ? 1 : 0);
    //printf("[P4: %d] ",     (reg_pos & 0x08) ? 1 : 0);
    //printf("[Prog: %d] ",   reg_prog ? 1 : 0);
    //printf("[Calib: %d]\n", reg_calib ? 1 : 0);

    if (reg_x & 0x01) {
        printf("-> Mover Esquerda\n");
        resetar_registrador(0x00);
    }
    if (reg_x & 0x02) {
        printf("-> Mover Direita\n");
        resetar_registrador(0x00);
    }
    if (reg_y & 0x01) {
        printf("-> Mover Cima\n");
        resetar_registrador(0x01);
    }
    if (reg_y & 0x02) {
        printf("-> Mover Baixo\n");
        resetar_registrador(0x01);
    }
    if (reg_pos & 0x01) {
        printf("-> Ir para Posição 1\n");
        resetar_registrador(0x02);
    }
    if (reg_pos & 0x02) {
        printf("-> Ir para Posição 2\n");
        resetar_registrador(0x02);
    }
    if (reg_pos & 0x04) {
        printf("-> Ir para Posição 3\n");
        resetar_registrador(0x02);
    }
    if (reg_pos & 0x08) {
        printf("-> Ir para Posição 4\n");
        resetar_registrador(0x02);
    }
    if (reg_prog) {
        printf("-> Programar Posição\n");
        resetar_registrador(0x03);
    }
    if (reg_calib) {
        printf("-> Calibrar Máquina\n");
        resetar_registrador(0x04);
    }
}

int main() {
    int pulsos_max_x, pulsos_max_y;

    raiox_init();              // inicia o pino do LED
    botoes_fisicos_init();     // inicia botões GPIO
    raiox_teste(); //Forca o estado de ligado para a led do raio X
    encoder_init();
    motores_init();

    calibrar_maquina(&pulsos_max_x, &pulsos_max_y);
    dashboard_set_calibracao( abs(pulsos_max_x), abs(pulsos_max_y) );

    while(1){
        verificar_botoes();    // painel touch
        ler_botoes_fisicos();  // GPIO debug
        //raiox_update();          // gerencia LED Raio-X
        encoder_print_pos();
        motor_atualizar_com_botoes();
        // Apenas para testar! Depois troque por valores reais dos encoders
        //dashboard_write_vx(0.2f);
        //dashboard_write_vy(0.4f);
        //dashboard_write_x(1.85f);
        //dashboard_write_y(0.8f);
        //dashboard_write_temp(23.5);  // Exemplo de temperatura em graus Celsius
        //dashboard_write_press(1013.25); // Exemplo de pressão em hPa
        //dashboard_write_estado(1);
        dashboard_envia_pos_vel_loop();
        usleep(50000);
    }

    return 0;
}