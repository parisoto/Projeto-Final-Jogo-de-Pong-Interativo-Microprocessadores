/* Includes ------------------------------------------------------------------*/
#include "st7789.h"
#include "main.h" // Se as variáveis HAL_HandleTypeDef não estiverem em st7789.h


/* Variáveis Externas --------------------------------------------------------*/
// Importa o handle da SPI que é inicializado no main.c
extern SPI_HandleTypeDef hspi1;


/* Funções do Driver ST7789 --------------------------------------------------*/


void ST7789_WriteCommand(uint8_t cmd) {
     ST7789_DC_LOW();
     ST7789_CS_LOW();
     HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
     ST7789_CS_HIGH();
 }

 void ST7789_WriteData(uint8_t data) {
     ST7789_DC_HIGH();
     ST7789_CS_LOW();
     HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
     ST7789_CS_HIGH();
 }

 void ST7789_Reset(void) {
     ST7789_RST_LOW();
     HAL_Delay(50);
     ST7789_RST_HIGH();
     HAL_Delay(50);
 }

 void ST7789_Init(void) {
     ST7789_Reset();

     // Sequência de inicialização mínima para ST7789 (240x320)

     // 1. Comando de saída do modo de suspensão
     ST7789_WriteCommand(0x11); // Sleep Out
     HAL_Delay(120);

     // 2. Controle de acesso à memória (MADCTL) - Ajuste a orientação aqui
     ST7789_WriteCommand(0x36);
     ST7789_WriteData(0xA0); // 0x00 para Posição Padrão (Vertical)

     // 3. Formato de Pixel (COLMOD) - 16 bits/pixel (RGB565)
     ST7789_WriteCommand(0x3A);
     ST7789_WriteData(0x55); // 0x55 = RGB565

     // 4. Comando de ligar o Display
     ST7789_WriteCommand(0x29); // Display ON
     HAL_Delay(10);
 }

 void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
     uint8_t data[4];

     // Column Address Set (CASSET)
     ST7789_WriteCommand(0x2A);
     data[0] = (x0 >> 8) & 0xFF; data[1] = x0 & 0xFF;
     data[2] = (x1 >> 8) & 0xFF; data[3] = x1 & 0xFF;
     ST7789_DC_HIGH(); ST7789_CS_LOW();
     HAL_SPI_Transmit(&hspi1, data, 4, HAL_MAX_DELAY);
     ST7789_CS_HIGH();

     // Row Address Set (RASET)
     ST7789_WriteCommand(0x2B);
     data[0] = (y0 >> 8) & 0xFF; data[1] = y0 & 0xFF;
     data[2] = (y1 >> 8) & 0xFF; data[3] = y1 & 0xFF;
     ST7789_DC_HIGH(); ST7789_CS_LOW();
     HAL_SPI_Transmit(&hspi1, data, 4, HAL_MAX_DELAY);
     ST7789_CS_HIGH();

     // Memory Write (RAMWR)
     ST7789_WriteCommand(0x2C);
 }

 void ST7789_FillColor(uint16_t color) {
     // A área total é a mesma: 240 * 320 = 76800 pixels
     uint32_t total_pixels = DISPLAY_WIDTH * DISPLAY_HEIGHT; // Usando as novas constantes
     uint8_t colorData[2] = { color >> 8, color & 0xFF };

     // Define a área completa (0,0) até (Largura-1, Altura-1)
     // Se a tela estiver em Horizontal (320x240): (0, 0) a (319, 239)
     ST7789_SetAddressWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

     // Enviar pixels
     ST7789_DC_HIGH();
     ST7789_CS_LOW();

     // ... (Seu loop otimizado ou não otimizado) ...
     for (uint32_t i = 0; i < total_pixels; i++) {
         HAL_SPI_Transmit(&hspi1, colorData, 2, 10);
     }
     ST7789_CS_HIGH();
 }

 //FUNÇÃO PARA A BOLINHA
 // Função que desenha um único pixel
 void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
     if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return; // Limites

     // Define uma janela de 1x1 no pixel (x, y)
     ST7789_SetAddressWindow(x, y, x, y);

     // Divide a cor em bytes e envia
     uint8_t colorData[2] = { color >> 8, color & 0xFF };
     ST7789_DC_HIGH();
     ST7789_CS_LOW();
     HAL_SPI_Transmit(&hspi1, colorData, 2, 10);
     ST7789_CS_HIGH();
 }

 void ST7789_DrawBall(uint16_t x_center, uint16_t y_center, uint16_t color) {
     // Calcula os cantos do quadrado baseado no centro e no raio
     uint16_t x0 = x_center - BALL_RADIUS;
     uint16_t y0 = y_center - BALL_RADIUS;
     uint16_t x1 = x_center + BALL_RADIUS;
     uint16_t y1 = y_center + BALL_RADIUS;

     // 1. Define a área do quadrado
     // Se o BALL_RADIUS for 6, a janela será de (Centro-6, Centro-6) até (Centro+6, Centro+6).
     ST7789_SetAddressWindow(x0, y0, x1, y1);

     // 2. Calcula o número de pixels
     // Note que (x1 - x0 + 1) é o lado do quadrado (ex: 13)
     uint32_t total_pixels = (uint32_t)(x1 - x0 + 1) * (uint32_t)(y1 - y0 + 1);
     uint8_t colorData[2] = { color >> 8, color & 0xFF };

     // 3. Envio dos dados de pixel
     ST7789_DC_HIGH();
     ST7789_CS_LOW();
     for (uint32_t i = 0; i < total_pixels; i++) {
         // Usa um timeout muito baixo para transmitir rapidamente
         HAL_SPI_Transmit(&hspi1, colorData, 2, 1);
     }
     ST7789_CS_HIGH();
 }

 void ST7789_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
 {
     // Calcula a largura (w) e altura (h) do retângulo
     uint16_t w = x1 - x0 + 1;
     uint16_t h = y1 - y0 + 1;

     // Calcula o número total de pixels na área
     uint32_t num_pixels = (uint32_t)w * h;

     // 1. Define a janela de endereçamento (o retângulo)
     ST7789_SetAddressWindow(x0, y0, x1, y1);

     // 2. Envia o comando RAM Write (para começar a enviar dados de cor)
     ST7789_WriteCommand(0x2C); // Comando "RAM Write"

     // 3. Define o pino DC para 'data' (para começar a enviar a cor)
     ST7789_DC_HIGH(); // Assumindo que ST7789_DC_HIGH() foi definido em st7789.h/main.h

     // 4. Prepara o valor da cor em 16-bits (2 bytes) para transmissão
     uint8_t high_byte = (uint8_t)(color >> 8); // Byte mais significativo
     uint8_t low_byte  = (uint8_t)(color & 0xFF);  // Byte menos significativo

     // 5. Entra no modo de transmissão de dados (CS LOW)
     ST7789_CS_LOW(); // Ativa o Chip Select

     // 6. Loop de envio da cor para cada pixel da janela
     for (uint32_t i = 0; i < num_pixels; i++)
     {
         // Envia o Byte Mais Significativo (MSB)
         HAL_SPI_Transmit(&hspi1, &high_byte, 1, 10); // Assumindo que hspi1 é o handle do seu SPI

         // Envia o Byte Menos Significativo (LSB)
         HAL_SPI_Transmit(&hspi1, &low_byte, 1, 10);
     }

     // 7. Sai do modo de transmissão de dados (CS HIGH)
     ST7789_CS_HIGH(); // Desativa o Chip Select
 }
