#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>  // <--- NOVO
#include "motores.h"
#include "botoes_fisicos.h"

// Definição dos GPIOs dos motores (BCM)
#define X_PWM   17
#define X_DIR1  27
#define X_DIR2  22

#define Y_PWM   23
#define Y_DIR1  24
#define Y_DIR2  25

#define DUTY_CYCLE 15  // ajuste aqui a intensidade do motor (0 a 100)

void motores_init() {
    wiringPiSetupGpio();

    pinMode(X_DIR1, OUTPUT);  pinMode(X_DIR2, OUTPUT);
    pinMode(Y_DIR1, OUTPUT);  pinMode(Y_DIR2, OUTPUT);

    digitalWrite(X_DIR1, LOW); digitalWrite(X_DIR2, LOW);
    digitalWrite(Y_DIR1, LOW); digitalWrite(Y_DIR2, LOW);

    // Inicializa PWM com duty inicial 0 e valor máximo 100
    softPwmCreate(X_PWM, 0, 100);
    softPwmCreate(Y_PWM, 0, 100);
}

// Motor X
static void motor_x_frente() {
    digitalWrite(X_DIR1, HIGH);
    digitalWrite(X_DIR2, LOW);
    softPwmWrite(X_PWM, DUTY_CYCLE);
    printf("[Motor X] Frente\n");
}

static void motor_x_reverso() {
    digitalWrite(X_DIR1, LOW);
    digitalWrite(X_DIR2, HIGH);
    softPwmWrite(X_PWM, DUTY_CYCLE);
    printf("[Motor X] Reverso\n");
}

static void motor_x_parar() {
    digitalWrite(X_DIR1, HIGH);  // freio ativo
    digitalWrite(X_DIR2, HIGH);
    softPwmWrite(X_PWM, 0);      // desliga PWM
    printf("[Motor X] Parado\n");
}

// Motor Y
static void motor_y_frente() {
    digitalWrite(Y_DIR1, HIGH);
    digitalWrite(Y_DIR2, LOW);
    softPwmWrite(Y_PWM, DUTY_CYCLE);
    printf("[Motor Y] Frente\n");
}

static void motor_y_reverso() {
    digitalWrite(Y_DIR1, LOW);
    digitalWrite(Y_DIR2, HIGH);
    softPwmWrite(Y_PWM, DUTY_CYCLE);
    printf("[Motor Y] Reverso\n");
}

static void motor_y_parar() {
    digitalWrite(Y_DIR1, HIGH);  // freio ativo
    digitalWrite(Y_DIR2, HIGH);
    softPwmWrite(Y_PWM, 0);      // desliga PWM
    printf("[Motor Y] Parado\n");
}

// Atualiza motores com base nos botões físicos
void motor_atualizar_com_botoes() {
    int cima  = botao_fisico_ativo(BTN_CIMA);
    int baixo = botao_fisico_ativo(BTN_BAIXO);
    int esq   = botao_fisico_ativo(BTN_ESQ);
    int dir   = botao_fisico_ativo(BTN_DIR);

    // Eixo Y
    if (cima)
        motor_y_frente();
    else if (baixo)
        motor_y_reverso();
    else
        motor_y_parar();

    // Eixo X
    if (dir)
        motor_x_frente();
    else if (esq)
        motor_x_reverso();
    else
        motor_x_parar();
}

int fim_curso_x_min() { return digitalRead(X_MIN_PIN) == 1; }
int fim_curso_x_max() { return digitalRead(X_MAX_PIN) == 1; }
int fim_curso_y_min() { return digitalRead(Y_MIN_PIN) == 1; }
int fim_curso_y_max() { return digitalRead(Y_MAX_PIN) == 1; }