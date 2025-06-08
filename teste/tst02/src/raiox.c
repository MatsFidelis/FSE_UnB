#include <stdio.h>
#include <wiringPi.h>
#include "raiox.h"
#include "botoes_fisicos.h"

#define PIN_RAIOX 18

static int estado_raiox = 0; // Estado inicial desligado

void raiox_init() {
    wiringPiSetupGpio();

    pinMode(PIN_RAIOX, OUTPUT);
    digitalWrite(PIN_RAIOX, LOW);
    estado_raiox = 0;
}

void raiox_set(int estado) {
    estado_raiox = estado ? 1 : 0;
    digitalWrite(PIN_RAIOX, estado_raiox ? HIGH : LOW);
}

void raiox_update() {
    if (botao_emergencia_ativo()) {
        raiox_set(0); // força desligar
        printf("[RaioX] Emergência ATIVADA - LED DESLIGADO\n");
    } else {
        printf("[RaioX] Estado atual: %s\n", estado_raiox ? "LIGADO" : "DESLIGADO");
    }
}

void raiox_teste() {
    raiox_set(1);
    printf("[RaioX - TESTE] Forçando LED LIGADO para teste\n");
}