/* Core/Inc/my_font.h */
#ifndef __MY_FONT_H
#define __MY_FONT_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h" // Se precisar de tipos como uint16_t ou uint8_t

// --- Definições da Fonte ---
#define FONT_WIDTH      5
#define FONT_HEIGHT     8
#define CHAR_START      32
#define CHAR_END        126
#define CHAR_COUNT      (CHAR_END - CHAR_START + 1)
#define CHAR_SIZE_BYTES FONT_WIDTH
#define ST7789_COLOR_TRANSPARENT 0xFFFF // Exemplo de 'cor' transparente

// --- Declaração Externa do Array de Dados ---
// O 'extern' diz ao compilador que este array existe, mas que os dados estão em outro lugar (.c)
extern const uint8_t MyFont_5x8[CHAR_COUNT][CHAR_SIZE_BYTES];

// --- Protótipos das Funções ---
void ST7789_DrawChar(uint16_t x, uint16_t y, char character, uint16_t fg_color, uint16_t bg_color);
void ST7789_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color);

// Protótipo para desenhar um caractere escalonado
void ST7789_DrawCharScaled(uint16_t x, uint16_t y, char character, uint16_t fg_color, uint16_t bg_color, uint8_t scale);

// Protótipo para desenhar uma string escalonada
void ST7789_DrawStringScaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale);

// ...

#ifdef __cplusplus
}
#endif

#endif /* __MY_FONT_H */
