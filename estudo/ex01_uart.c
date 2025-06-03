#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

// ===== MATRÍCULA (73184) =============================================================
unsigned char matricula[] = {7, 3, 1, 8, 4};

// ===== PROTÓTIPOS ====================================================================
int inicializa_uart();
void fecha_uart(int uart_fd);
int envia_comando(int uart_fd, unsigned char comando);
void le_resposta_string(int uart_fd);
void le_resposta_inteiro(int uart_fd);
void le_resposta_float(int uart_fd);

// ===== IMPLEMENTAÇÃO ==================================================================

int inicializa_uart() {
        int uart_fd = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
        if (uart_fd == -1) {
                perror("Erro ao abrir UART");
                return -1;
        }

        struct termios options;
        tcgetattr(uart_fd, &options);
        options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart_fd, TCIFLUSH);
        tcsetattr(uart_fd, TCSANOW, &options);

        return uart_fd;
}

void fecha_uart(int uart_fd) {
        close(uart_fd);
}

int envia_comando(int uart_fd, unsigned char comando) {
        unsigned char pacote[6];
        pacote[0] = comando;
        memcpy(&pacote[1], matricula, sizeof(matricula));

        int bytes_enviados = write(uart_fd, pacote, sizeof(pacote));
        if (bytes_enviados < 0) {
                perror("Erro ao escrever na UART");
                return -1;
        }
        return bytes_enviados;
}

void le_resposta_string(int uart_fd) {
        unsigned char tamanho;
        int lidos = read(uart_fd, &tamanho, 1);
        if (lidos <= 0) {
                printf("Erro ou nenhuma resposta recebida.\n");
                return;
        }

        unsigned char resposta[tamanho + 1];
        lidos = read(uart_fd, resposta, tamanho);
        if (lidos > 0) {
                resposta[lidos] = '\0';
                printf("Resposta recebida (string): %s\n", resposta);
        } else {
                printf("Erro ao ler string.\n");
        }
}

void le_resposta_inteiro(int uart_fd) {
        unsigned char buffer[4];
        int bytes_lidos = read(uart_fd, buffer, 4);

        if (bytes_lidos < 4) {
                printf("Erro ou dados insuficientes. Lidos: %d\n", bytes_lidos);
                return;
        }

        printf("Bytes recebidos (inteiro): ");
        for (int i = 0; i < 4; i++) {
                printf("0x%02X ", buffer[i]);
        }
        printf("\n");

        int valor;
        memcpy(&valor, buffer, sizeof(int));
        printf("Valor inteiro reconstruído: %d\n", valor);
}

void le_resposta_float(int uart_fd) {
        unsigned char buffer[4];
        int bytes_lidos = read(uart_fd, buffer, 4);

        if (bytes_lidos < 4) {
                printf("Erro ou dados insuficientes. Lidos: %d\n", bytes_lidos);
                return;
        }

        printf("Bytes recebidos (float): ");
        for (int i = 0; i < 4; i++) {
                printf("0x%02X ", buffer[i]);
        }
        printf("\n");

        float valor;
        memcpy(&valor, buffer, sizeof(float));
        printf("Valor float reconstruído: %.2f\n", valor);
}

// ===== MAIN (altere manualmente aqui para testar) ===================================================
int main() {
        int uart_fd = inicializa_uart();        // Inicializo a UART
        if (uart_fd == -1) return 1;

        /*
        // Exemplo de teste: solicitação de STRING
        printf("Solicitando string (comando 0xA3)...\n");
        if (envia_comando(uart_fd, 0xA3) > 0) {
        usleep(200000);  // Aguarda 200 ms
        le_resposta_string(uart_fd);
        }
        */

        // Exemplo de teste: solicitação de INTEIRO
        /*
           printf("Solicitando inteiro (comando 0xA1)...\n");
           if (envia_comando(uart_fd, 0xA1) > 0) {
           usleep(200000);
           le_resposta_inteiro(uart_fd);
           }
           */

        // Exemplo de teste: solicitação de FLOAT

        printf("Solicitando float (comando 0xA2)...\n");
        if (envia_comando(uart_fd, 0xA2) > 0) {
                usleep(200000);
                le_resposta_float(uart_fd);
        }


        fecha_uart(uart_fd);    // Fecho comunicação na UART
        return 0;
}


// UART nao se importa com o pacote que esta sendo enviado, as configurações adicionais que crio para categorizar um pacote, como memcpy e afins, 
// fazem parte das etapas adicionais do modbus. Cada responsabilidade está em um modulo diferente, e no trabalho ir aos poucos, realizando boas praticas, criando 
// uma camada por vez, para ir adicionando.

// Opções para lidar com bugs e erros na atualização de estados, é a realização de flush no buffer, para se atualizar o estado

