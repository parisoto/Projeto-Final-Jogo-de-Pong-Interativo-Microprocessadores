#include "joystick.h"

ADC_HandleTypeDef *joystick_adc;

// Faixa de "zona morta" (ajuste depois)
#define JOYSTICK_CENTER_LOW   1800
#define JOYSTICK_CENTER_HIGH  2300

void Joystick_Init(ADC_HandleTypeDef *hadc)
{
    joystick_adc = hadc;
}

uint16_t Joystick_Read(void)
{
    HAL_ADC_Start(joystick_adc);
    HAL_ADC_PollForConversion(joystick_adc, HAL_MAX_DELAY);
    uint16_t val = HAL_ADC_GetValue(joystick_adc);
    HAL_ADC_Stop(joystick_adc);
    return val;
}

int16_t Joystick_GetPosition(uint16_t raw)
{
    return (int16_t)raw - 2048;  // centro aproximado (ADC 12 bits)
}

/**
 * 0 = parado
 * 1 = cima
 * 2 = baixo
 */
uint8_t Joystick_GetState(void)
{
    uint16_t val = Joystick_Read();

    if (val < JOYSTICK_CENTER_LOW)
    {
        return 1; // cima
    }
    else if (val > JOYSTICK_CENTER_HIGH)
    {
        return 2; // baixo
    }

    return 0; // centro
}
