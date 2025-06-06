#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include "crc16.h"

#define UART_PATH "/dev/serial0"
#define MATR_0 2
#define MATR_1 9
#define MATR_2 0
#define MATR_3 7
#define COD_SOLIC_INT 0xA1
#define COD_SOLIC_FLOAT 0xA2
#define COD_SOLIC_STR 0xA3
#define COD_ENV_INT 0xB1
#define COD_ENV_FLOAT 0xB2
#define COD_ENV_STR 0xB3
#define END_DISP 0x01
#define COD_FUNC_SOLIC 0x23
#define COD_FUNC_ENV 0x16


int open_uart();
int write_uart(unsigned char* buffer, int length);
int read_uart(unsigned char* buffer, int max_length);
void solicitar_inteiro();
void solicitar_float();
void solicitar_string();
void enviar_inteiro();
void enviar_float();
void enviar_string();

int main() {
    int opcao;
    do {
        printf("\n--- MENU ---\n");
        printf("1. Solicitar inteiro\n");
        printf("2. Solicitar float\n");
        printf("3. Solicitar string\n");
        printf("4. Enviar inteiro\n");
        printf("5. Enviar float\n");
        printf("6. Enviar string\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        int uart_fd = open_uart();
        switch (opcao) {
            case 1: solicitar_inteiro(); break;
            case 2: solicitar_float(); break;
            case 3: solicitar_string(); break;
            case 4: enviar_inteiro(); break;
            case 5: enviar_float(); break;
            case 6: enviar_string(); break;
            case 0: printf("Encerrando...\n"); break;
            default: printf("Opção inválida!\n");
        }
        close(uart_fd);

    } while (opcao != 0);

    return 0;
}

// Função para abrir a UART
int open_uart() {
    int uart_fd = open(UART_PATH, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) {
        perror("Erro ao abrir a UART");
        return -1;
    }

    struct termios options;
    tcgetattr(uart_fd, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);

    return uart_fd;
}


// Função para escrever na UART
int write_uart(unsigned char* buffer, int length) {
    int uart_fd = open_uart();
    if (uart_fd == -1) return -1;

    int count = write(uart_fd, buffer, length);
    close(uart_fd);
    return count;
}

// Função para ler da UART
int read_uart(unsigned char* buffer, int max_length) {
    int uart_fd = open_uart();
    if (uart_fd == -1) return -1;

    usleep(200000); // Aguarda 200ms
    usleep(200000); // Aguarda 200ms

    int rx_length = read(uart_fd, buffer, max_length);
    close(uart_fd);
    return rx_length;
}

// Funções específicas de comandos
void solicitar_inteiro() {
    unsigned char msg_sem_CRC[] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_INT, MATR_0, MATR_1, MATR_2, MATR_3};
    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));

    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[7 + 2] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_INT, MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg[7], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");

    write_uart(msg, sizeof(msg));

    unsigned char buffer[4];
    int len = read_uart(buffer, 4);
    if (len == 4) {
        int valor;
        memcpy(&valor, buffer, 4);
        printf("Valor inteiro recebido: %d\n", valor);
    } else {
        printf("Erro na leitura do inteiro.\n");
    }

}

void solicitar_float() {
    unsigned char msg_sem_CRC[] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_FLOAT, MATR_0, MATR_1, MATR_2, MATR_3};
    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));
    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[7 + 2] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_FLOAT, MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg[7], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");

    write_uart(msg, sizeof(msg));

    unsigned char buffer[4];
    int len = read_uart(buffer, 4);
    if (len == 4) {
        float valor;
        memcpy(&valor, buffer, 4);
        printf("Valor float recebido: %.2f\n", valor);
    } else {
        printf("Erro na leitura do float.\n");
    }
}

void solicitar_string() {
    unsigned char msg_sem_CRC[] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_STR, MATR_0, MATR_1, MATR_2, MATR_3};
    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));
    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[7 + 2] = {END_DISP, COD_FUNC_SOLIC, COD_SOLIC_STR, MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg[7], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");
    
    write_uart(msg, sizeof(msg));

    unsigned char buffer[256];
    int len = read_uart(buffer, 256);

    if (len > 1) {
        int tam = buffer[0];
        char str[256] = {0};
        memcpy(str, &buffer[1], tam);
        printf("String recebida (%d bytes): %s\n", tam, str);
    } else {
        printf("Erro na leitura da string.\n");
    }
}

void enviar_inteiro() {
    int valor;
    printf("Digite um inteiro: ");
    scanf("%d", &valor);

    unsigned char msg_sem_CRC[3 + 4 + 4] = {END_DISP, COD_FUNC_ENV, COD_ENV_INT};
    memcpy(&msg_sem_CRC[3], &valor, 4);
    unsigned char mat[] = {MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg_sem_CRC[7], mat, 4);

    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));
    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[11 + 2] = {};
    memcpy(&msg[0], &msg_sem_CRC, 11);
    memcpy(&msg[11], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");



    write_uart(msg, sizeof(msg));

    unsigned char buffer[4];
    int len = read_uart(buffer, 4);
    if (len == 4) {
        int echo;
        memcpy(&echo, buffer, 4);
        printf("Inteiro enviado, retorno recebido: %d\n", echo);
    } else {
        printf("Erro no envio do inteiro.\n");
    }
}

void enviar_float() {
    float valor;
    printf("Digite um número float: ");
    scanf("%f", &valor);

    unsigned char msg_sem_CRC[3 + 4 + 4] = {END_DISP, COD_FUNC_ENV, COD_ENV_FLOAT};
    memcpy(&msg_sem_CRC[3], &valor, 4);
    unsigned char mat[] = {MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg_sem_CRC[7], mat, 4);

    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));
    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[11 + 2];
    memcpy(&msg[0], &msg_sem_CRC, 11);
    memcpy(&msg[11], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");


    write_uart(msg, sizeof(msg));

    unsigned char buffer[4];
    int len = read_uart(buffer, 4);
    if (len == 4) {
        float echo;
        memcpy(&echo, buffer, 4);
        printf("Float enviado, retorno recebido: %.2f\n", echo);
    } else {
        printf("Erro no envio do float.\n");
    }
}

void enviar_string() {
    char str[250];
    printf("Digite a string para enviar (max 250 chars): ");
    scanf(" %[^\n]", str);

    int tam = strlen(str);
    printf("Tamanho da string: %d bytes\n", tam);

    unsigned char msg_sem_CRC[3 + 1 + tam + 4];
    msg_sem_CRC[0] = END_DISP;
    msg_sem_CRC[1] = COD_FUNC_ENV;
    msg_sem_CRC[2] = COD_ENV_STR;
    msg_sem_CRC[3] = tam;
    memcpy(&msg_sem_CRC[4], str, tam);
    unsigned char mat[] = {MATR_0, MATR_1, MATR_2, MATR_3};
    memcpy(&msg_sem_CRC[4 + tam], mat, 4);

    short crc16 = calcula_CRC(msg_sem_CRC, sizeof(msg_sem_CRC));
    printf("Mensagem sem CRC: ");
    for (size_t i = 0; i < sizeof(msg_sem_CRC); i++) {
        printf("%02X ", msg_sem_CRC[i]);
    }
    printf("\n");
    printf("CRC16: 0x%04X\n", (unsigned short)crc16);

    unsigned char msg[8 + tam + 2];
    memcpy(&msg[0], &msg_sem_CRC, 8 + tam);
    memcpy(&msg[8 + tam], &crc16, 2);
    printf("Mensagem com CRC: ");
    for (size_t i = 0; i < sizeof(msg); i++) {
        printf("%02X ", msg[i]);
    }
    printf("\n");


    write_uart(msg, sizeof(msg));

    unsigned char buffer[256];
    int len = read_uart(buffer, 256);

    if (len > 1) {
        int tam = buffer[0];
        char recv[256] = {0};
        memcpy(recv, &buffer[1], tam);
        printf("String enviada, string retorno recebida: %s\n", recv);
    } else {
        printf("Erro no envio da string.\n");
    }
}

