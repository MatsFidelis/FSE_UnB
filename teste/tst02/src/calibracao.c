#include "calibracao.h"
#include "motores.h"
#include "encoder.h"
#include <unistd.h>
#include <stdio.h>
#include "dashboard.h"

void calibrar_maquina(int* pulsos_max_x, int* pulsos_max_y) {
    // 1) avisa à dashboard que calibração começou
    dashboard_write_estado(1);

    DUTY_CYCLE = 85;
    printf("Calibrando: indo para a posição zero (min)...\n");

    // 1. Ambos os eixos vão para o MIN ao mesmo tempo
    int x_ok = 0, y_ok = 0;
    while (!(x_ok && y_ok)) {
        if (!x_ok) {
            if (!fim_curso_x_min()) {
                motor_x_reverso();
            } else {
                motor_x_parar();
                x_ok = 1;
            }
        }
        if (!y_ok) {
            if (!fim_curso_y_min()) {
                motor_y_reverso();
            } else {
                motor_y_parar();
                y_ok = 1;
            }
        }
        usleep(3000);
    }
    motor_x_parar();
    motor_y_parar();
    encoder_reset_posX();
    encoder_reset_posY();
    usleep(50000);

    printf("Calibrando: indo para o máximo dos dois eixos...\n");

    // 2. Ambos vão para o MAX ao mesmo tempo
    x_ok = 0; y_ok = 0;
    while (!(x_ok && y_ok)) {
        if (!x_ok) {
            if (!fim_curso_x_max()) {
                motor_x_frente();
            } else {
                motor_x_parar();
                x_ok = 1;
            }
        }
        if (!y_ok) {
            if (!fim_curso_y_max()) {
                motor_y_frente();
            } else {
                motor_y_parar();
                y_ok = 1;
            }
        }
        usleep(3000);
    }
    motor_x_parar();
    motor_y_parar();
    *pulsos_max_x = encoder_get_posX();
    *pulsos_max_y = encoder_get_posY();
    printf("X calibrado: %d pulsos, Y calibrado: %d pulsos\n", *pulsos_max_x, *pulsos_max_y);
    usleep(50000);

    printf("Retornando ambos para o zero...\n");

    // 3. Ambos voltam ao MIN juntos
    x_ok = 0; y_ok = 0;
    while (!(x_ok && y_ok)) {
        if (!x_ok) {
            if (!fim_curso_x_min()) {
                motor_x_reverso();
            } else {
                motor_x_parar();
                x_ok = 1;
            }
        }
        if (!y_ok) {
            if (!fim_curso_y_min()) {
                motor_y_reverso();
            } else {
                motor_y_parar();
                y_ok = 1;
            }
        }
        usleep(3000);
    }
    motor_x_parar();
    motor_y_parar();
    encoder_reset_posX();
    encoder_reset_posY();
    usleep(50000);
    DUTY_CYCLE = 100;
    printf("Calibração concluída: X=%d pulsos, Y=%d pulsos (posição atual = zero)\n", *pulsos_max_x, *pulsos_max_y);

    // 2) avisa à dashboard que calibração terminou
    dashboard_write_estado(0);

}