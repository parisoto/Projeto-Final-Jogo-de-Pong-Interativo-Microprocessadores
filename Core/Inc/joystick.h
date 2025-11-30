#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "main.h"

// Inicializa o joystick com o ADC configurado no CubeMX
void Joystick_Init(ADC_HandleTypeDef *hadc);

// Lê um valor RAW do ADC (0–4095)
uint16_t Joystick_Read(void);

// Converte RAW em posição (+100 / 0 / -100)
int16_t Joystick_GetPosition(uint16_t raw);

// Retorna:
// 0 = parado
// 1 = cima
// 2 = baixo
uint8_t Joystick_GetState(void);

#endif
