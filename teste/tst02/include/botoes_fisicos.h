#ifndef BOTOES_FISICOS_H
#define BOTOES_FISICOS_H

// Definições dos GPIOs dos botões físicos (BCM)
#define BTN_CIMA    16
#define BTN_BAIXO   1
#define BTN_ESQ     7
#define BTN_DIR     8
#define BTN_EMER    11

void botoes_fisicos_init();
void ler_botoes_fisicos();
int botao_emergencia_ativo();
int botao_fisico_ativo(int gpio);

#endif