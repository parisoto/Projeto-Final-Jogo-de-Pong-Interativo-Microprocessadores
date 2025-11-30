#ifndef __ST7789_H
#define __ST7789_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h" // Necessário para HAL_SPI_Transmit e GPIO defs

/* Private defines -----------------------------------------------------------*/
 // Cores Primárias e Básicas (R, G, B, W, K)
 #define ST7789_COLOR_RED        0xF800  // R cheio (31), G zero, B zero
 #define ST7789_COLOR_BLUE       0x001F  // R zero, G zero, B cheio (31)
 #define ST7789_COLOR_GREEN      0x07E0  // R zero, G cheio (63), B zero
 #define ST7789_COLOR_WHITE      0xFFFF  // Todos cheios
 #define ST7789_COLOR_BLACK      0x0000  // Todos zero
 #define ST7789_COLOR_CYAN       0x07FF  // GREEN + BLUE (0x07E0 | 0x001F)
 #define ST7789_COLOR_MAGENTA    0xF81F  // RED + BLUE (0xF800 | 0x001F)
 #define ST7789_COLOR_YELLOW     0xFFE0  // RED + GREEN (0xF800 | 0x07E0)

 // Tons de Cinza (Cinco tons de cinza escuro a claro)
 #define ST7789_COLOR_DARKGREY   0x7BEF  // ~50%
 #define ST7789_COLOR_GREY       0x8410  // ~60%
 #define ST7789_COLOR_LIGHTGREY  0xC618  // ~80%

 // Outras Cores Comuns
 #define ST7789_COLOR_ORANGE     0xFD20  // RED + Um pouco de GREEN
 #define ST7789_COLOR_PINK       0xF81E  // RED + B (Quase magenta)
 #define ST7789_COLOR_PURPLE     0xA11F  // Um pouco menos de RED que magenta
 #define ST7789_COLOR_BROWN      0xA145  // Tons terrosos
 #define ST7789_COLOR_NAVY       0x000F  // Azul Marinho (B forte, G e R baixo)

 // Definições de controle do Display ST7789
 #define ST7789_CS_LOW()   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
 #define ST7789_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
 #define ST7789_DC_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
 #define ST7789_DC_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)
 #define ST7789_RST_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
 #define ST7789_RST_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

 //Definicao do tamanho da bola
#define BALL_RADIUS 3

 // Dimensões do Display
 #define DISPLAY_WIDTH   320
 #define DISPLAY_HEIGHT  240


 /* Funções Públicas (Protótipos) ---------------------------------------------*/

 void ST7789_WriteCommand(uint8_t cmd);
 void ST7789_WriteData(uint8_t data);
 void ST7789_Reset(void);
 void ST7789_Init(void);
 void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
 void ST7789_FillColor(uint16_t color);
 void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
 void ST7789_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

 // Funções específicas do seu código
 void ST7789_DrawBall(uint16_t x_center, uint16_t y_center, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* __ST7789_H */
