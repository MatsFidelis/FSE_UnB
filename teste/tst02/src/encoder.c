#include <stdio.h>
#include <wiringPi.h>
#include "encoder.h"

// Pinos GPIO (BCM) para os sinais A e B dos encoders
#define ENC_X_A  5
#define ENC_X_B  6
#define ENC_Y_A  12
#define ENC_Y_B  13

static volatile int posX = 0;
static volatile int posY = 0;

// Tratamento das interrupções
void isr_encoder_x_a() {
    if (digitalRead(ENC_X_A) == digitalRead(ENC_X_B))
        posX++;
    else
        posX--;
}

void isr_encoder_y_a() {
    if (digitalRead(ENC_Y_A) == digitalRead(ENC_Y_B))
        posY++;
    else
        posY--;
}


void encoder_init() {
    wiringPiSetupGpio();

    pinMode(ENC_X_A, INPUT);
    pinMode(ENC_X_B, INPUT);
    pinMode(ENC_Y_A, INPUT);
    pinMode(ENC_Y_B, INPUT);

    pullUpDnControl(ENC_X_A, PUD_UP);
    pullUpDnControl(ENC_X_B, PUD_UP);
    pullUpDnControl(ENC_Y_A, PUD_UP);
    pullUpDnControl(ENC_Y_B, PUD_UP);

    wiringPiISR(ENC_X_A, INT_EDGE_BOTH, &isr_encoder_x_a);
    wiringPiISR(ENC_Y_A, INT_EDGE_BOTH, &isr_encoder_y_a);
}

int encoder_get_posX() {
    return posX;
}

int encoder_get_posY() {
    return posY;
}

void encoder_print_pos() {
    printf("[ENCODER] X = %d | Y = %d\n", encoder_get_posX(), encoder_get_posY());
}

void encoder_reset_posX() { posX = 0; }
void encoder_reset_posY() { posY = 0; }