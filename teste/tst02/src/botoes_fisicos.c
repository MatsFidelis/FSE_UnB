#include <stdio.h>
#include <wiringPi.h>
#include "botoes_fisicos.h"

// Definição dos pinos para cada botão físico
#define BTN_CIMA    16
#define BTN_BAIXO   1
#define BTN_ESQ     7
#define BTN_DIR     8
#define BTN_EMER    11

// Função para configurar os pinos
void botoes_fisicos_init() {
    wiringPiSetupGpio();

    pinMode(BTN_CIMA,    INPUT);
    pinMode(BTN_BAIXO,   INPUT);
    pinMode(BTN_ESQ,     INPUT);
    pinMode(BTN_DIR,     INPUT);
    pinMode(BTN_EMER,    INPUT);

    pullUpDnControl(BTN_CIMA,    PUD_UP);
    pullUpDnControl(BTN_BAIXO,   PUD_UP);
    pullUpDnControl(BTN_ESQ,     PUD_UP);
    pullUpDnControl(BTN_DIR,     PUD_UP);
    pullUpDnControl(BTN_EMER,    PUD_UP);
}

// Função para ler e imprimir os estados
void ler_botoes_fisicos() {
    printf("[Botões Físicos] ");
    printf("Cima: %s | ",    digitalRead(BTN_CIMA)  == HIGH ? "ATIVADO" : "DESATIVADO");
    printf("Baixo: %s | ",   digitalRead(BTN_BAIXO) == HIGH ? "ATIVADO" : "DESATIVADO");
    printf("Esq: %s | ",     digitalRead(BTN_ESQ)   == HIGH ? "ATIVADO" : "DESATIVADO");
    printf("Dir: %s | ",     digitalRead(BTN_DIR)   == HIGH ? "ATIVADO" : "DESATIVADO");
    printf("Emer: %s \n",    digitalRead(BTN_EMER)  == HIGH ? "ATIVADO" : "DESATIVADO");
}

// Função para verificar se o botão de emergência está pressionado
int botao_emergencia_ativo() {
    return digitalRead(BTN_EMER) == HIGH;
}

int botao_fisico_ativo(int gpio) {
    return digitalRead(gpio) == HIGH;
}