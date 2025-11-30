/* Core/Src/my_font.c */

#include "my_font.h"
#include "main.h"

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240

// =========================================================
// 1. DADOS DA FONTE (O bitmap 5x8)
// =========================================================
// Definição do array de dados da fonte.
// Bit 0 = Topo (Y=0); Bit 7 = Base (Y=7). 5 Bytes por caractere.
const uint8_t MyFont_5x8[CHAR_COUNT][CHAR_SIZE_BYTES] = {
    // ----------------------------------------------------
    // ASCII 32 - 47: Espaço, Símbolos Comuns
    // ----------------------------------------------------
    // 32 (Espaço)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // 33 (!)
	{0x00, 0x00, 0x00, 0x00, 0x5F},
    // 34 (")
    {0x06, 0x00, 0x06, 0x00, 0x00},
    // 35 (#)
    {0x24, 0x7E, 0x24, 0x7E, 0x24},
    // 36 ($)
    {0x24, 0x54, 0x7F, 0x54, 0x48},
    // 37 (%)
    {0x44, 0x4C, 0x10, 0x26, 0x19},
    // 38 (&)
    {0x36, 0x49, 0x56, 0x20, 0x50},
    // 39 (')
    {0x04, 0x02, 0x00, 0x00, 0x00},
    // 40 (()
    {0x00, 0x7E, 0x09, 0x01, 0x00},
    // 41 ())
    {0x00, 0x01, 0x09, 0x7E, 0x00},
    // 42 (*)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // 43 (+)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // 44 (,)
    {0x60, 0x30, 0x00, 0x00, 0x00},
    // 45 (-)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // 46 (.)
    {0x60, 0x00, 0x00, 0x00, 0x00},
    // 47 (/)
    {0x60, 0x30, 0x18, 0x0C, 0x06},

    // ----------------------------------------------------
    // ASCII 48 - 57: Números (0-9)
    // ----------------------------------------------------
    // 48 (0)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // 49 (1)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 50 (2)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 51 (3)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 52 (4)
	{0x18, 0x14, 0x12, 0x7F, 0x10},
	//{0x10, 0x28, 0x24, 0xFE, 0x20},
    // 53 (5)
	{0x27, 0x25, 0x25, 0x25, 0x3D},
	// 54 (6)
    {0x3E, 0x49, 0x49, 0x49, 0x20},
    // 55 (7)
	{0x01, 0x01, 0x01, 0x09, 0x7F},
    // 56 (8)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 57 (9)
    {0x06, 0x49, 0x49, 0x49, 0x3E},

    // ----------------------------------------------------
    // ASCII 58 - 64: Pontuação e Símbolos
    // ----------------------------------------------------
    // 58 (:)
    {0x6C, 0x00, 0x00, 0x00, 0x00},
    // 59 (;)
    {0x6C, 0x30, 0x00, 0x00, 0x00},
    // 60 (<)
    {0x08, 0x14, 0x22, 0x41, 0x00},
    // 61 (=)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // 62 (>)
    {0x41, 0x22, 0x14, 0x08, 0x00},
    // 63 (?)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // 64 (@)
    {0x3E, 0x41, 0x5D, 0x55, 0x3E},

    // ----------------------------------------------------
    // ASCII 65 - 90: Letras Maiúsculas (A-Z)
    // ----------------------------------------------------
    // 65 (A)
    {0x7E, 0x09, 0x09, 0x09, 0x7E},
    // 66 (B)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // 67 (C)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // 68 (D)
    {0x7F, 0x41, 0x41, 0x41, 0x3E},
    // 69 (E)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // 70 (F)
    {0x7F, 0x09, 0x09, 0x01, 0x01},
    // 71 (G)
    {0x3E, 0x41, 0x51, 0x49, 0x7A},
    // 72 (H)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // 73 (I)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // 74 (J)
    {0x30, 0x40, 0x40, 0x40, 0x7F},
    // 75 (K)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // 76 (L)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // 77 (M)
    {0x7F, 0x02, 0x04, 0x02, 0x7F},
    // 78 (N)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // 79 (O)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // 80 (P)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // 81 (Q)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // 82 (R)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // 83 (S)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // 84 (T)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // 85 (U)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // 86 (V)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // 87 (W)
    {0x3F, 0x40, 0x30, 0x40, 0x3F},
    // 88 (X)
    {0x41, 0x22, 0x1C, 0x22, 0x41},
    // 89 (Y)
    {0x01, 0x02, 0x7C, 0x02, 0x01},
    // 90 (Z)
    {0x41, 0x61, 0x51, 0x49, 0x47},

    // ----------------------------------------------------
    // ASCII 91 - 96: Colchetes e Símbolos
    // ----------------------------------------------------
    // 91 ([)
    {0x7F, 0x41, 0x41, 0x00, 0x00},
    // 92 (\)
    {0x06, 0x0C, 0x18, 0x30, 0x60},
    // 93 (])
    {0x00, 0x41, 0x41, 0x7F, 0x00},
    // 94 (^)
    {0x04, 0x02, 0x01, 0x02, 0x04},
    // 95 (_)
    {0x80, 0x80, 0x80, 0x80, 0x80},
    // 96 (`)
    {0x00, 0x04, 0x02, 0x00, 0x00},

    // ----------------------------------------------------
    // ASCII 97 - 122: Letras Minúsculas (a-z)
    // ----------------------------------------------------
    // 97 (a)
    {0x20, 0x54, 0x54, 0x54, 0x78},
    // 98 (b)
    {0x7F, 0x24, 0x24, 0x24, 0x18},
    // 99 (c)
    {0x38, 0x44, 0x44, 0x44, 0x00},
    // 100 (d)
    {0x18, 0x24, 0x24, 0x24, 0x7F},
    // 101 (e)
    {0x38, 0x54, 0x54, 0x54, 0x18},
    // 102 (f)
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    // 103 (g)
    {0x48, 0x54, 0x54, 0x54, 0x3F},
    // 104 (h)
    {0x7F, 0x04, 0x04, 0x04, 0x78},
    // 105 (i)
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    // 106 (j)
    {0x20, 0x40, 0x40, 0x40, 0x7F},
    // 107 (k)
    {0x7F, 0x10, 0x28, 0x44, 0x00},
    // 108 (l)
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    // 109 (m)
    {0x7C, 0x04, 0x18, 0x04, 0x7C},
    // 110 (n)
    {0x7C, 0x04, 0x04, 0x04, 0x78},
    // 111 (o)
    {0x38, 0x44, 0x44, 0x44, 0x38},
    // 112 (p)
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    // 113 (q)
    {0x08, 0x14, 0x14, 0x14, 0x7C},
    // 114 (r)
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    // 115 (s)
    {0x48, 0x54, 0x54, 0x54, 0x08},
    // 116 (t)
    {0x04, 0x04, 0x7F, 0x44, 0x00},
    // 117 (u)
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    // 118 (v)
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    // 119 (w)
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    // 120 (x)
    {0x44, 0x28, 0x10, 0x28, 0x44},
    // 121 (y)
    {0x44, 0x28, 0x10, 0x28, 0x7C},
    // 122 (z)
    {0x44, 0x64, 0x54, 0x4C, 0x44},

    // ----------------------------------------------------
    // ASCII 123 - 126: Chaves e Til
    // ----------------------------------------------------
    // 123 ({)
    {0x08, 0x7E, 0x41, 0x00, 0x00},
    // 124 (|)
    {0x7F, 0x00, 0x00, 0x00, 0x00},
    // 125 (})
    {0x00, 0x41, 0x7E, 0x08, 0x00},
    // 126 (~)
    {0x00, 0x08, 0x04, 0x08, 0x00}
};

// =========================================================
// 2. IMPLEMENTAÇÃO DAS FUNÇÕES
// =========================================================

// Inclua a função ST7789_DrawPixel ou declare ela como extern em my_font.h se ela estiver em main.c
extern void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);


void ST7789_DrawChar(uint16_t x, uint16_t y, char character, uint16_t fg_color, uint16_t bg_color) {

    if (character < CHAR_START || character > CHAR_END) {
        character = CHAR_START; // Usa ESPAÇO se o caractere for inválido
    }

    uint8_t font_index = character - CHAR_START;
    const uint8_t *char_data = MyFont_5x8[font_index];

    uint8_t col, row;

    for (col = 0; col < FONT_WIDTH; col++) {
        uint8_t data_byte = char_data[col];

        for (row = 0; row < FONT_HEIGHT; row++) {

            uint16_t current_x = x + col;
            uint16_t current_y = y + row;

            // Verifica se o bit do pixel está ligado (1)
            if ((data_byte >> row) & 0x01) {
                ST7789_DrawPixel(current_x, current_y, fg_color);
            } else {
                // Desenha a cor de fundo apenas se não for transparente
                if (bg_color != ST7789_COLOR_TRANSPARENT) {
                    ST7789_DrawPixel(current_x, current_y, bg_color);
                }
            }
        }
    }
}

void ST7789_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color) {
    uint16_t current_x = x;

    while (*str) {
        ST7789_DrawChar(current_x, y, *str, fg_color, bg_color);

        // Largura da fonte (5) + 1 pixel de espaço
        current_x += (FONT_WIDTH + 1);

        if (current_x + FONT_WIDTH >= DISPLAY_WIDTH) {
            break;
        }

        str++;
    }
}

// Função para desenhar um caractere escalonado (ampliado)
void ST7789_DrawCharScaled(uint16_t x, uint16_t y, char character, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {

    if (scale == 0) scale = 1; // Não permite escala zero

    if (character < CHAR_START || character > CHAR_END) {
        character = CHAR_START;
    }

    uint8_t font_index = character - CHAR_START;
    const uint8_t *char_data = MyFont_5x8[font_index];

    uint8_t col, row;
    uint8_t i, j;

    // Itera sobre as colunas da fonte (5 colunas)
    for (col = 0; col < FONT_WIDTH; col++) {
        uint8_t data_byte = char_data[col];

        // Itera sobre as linhas (pixels verticais) do caractere (8 linhas)
        for (row = 0; row < FONT_HEIGHT; row++) {

            uint16_t current_x_start = x + (col * scale);
            uint16_t current_y_start = y + (row * scale);

            // Verifica se o bit do pixel está ligado (1)
            if ((data_byte >> row) & 0x01) {
                // Pixel ligado: usa a cor de primeiro plano (Foreground)
                uint16_t color = fg_color;

                // Desenha um BLOCO de 'scale' x 'scale' pixels
                for (i = 0; i < scale; i++) {
                    for (j = 0; j < scale; j++) {
                        ST7789_DrawPixel(current_x_start + i, current_y_start + j, color);
                    }
                }
            } else {
                // Pixel desligado: usa a cor de fundo (Background)
                if (bg_color != ST7789_COLOR_TRANSPARENT) {
                    uint16_t color = bg_color;

                    // Desenha um BLOCO de 'scale' x 'scale' pixels
                    for (i = 0; i < scale; i++) {
                        for (j = 0; j < scale; j++) {
                            ST7789_DrawPixel(current_x_start + i, current_y_start + j, color);
                        }
                    }
                }
            }
        }
    }
    // Opcional: Para evitar artefatos em fundos transparentes, você pode preencher
    // o espaço extra entre colunas (coluna 5) se a escala for grande.
}

// Função para desenhar uma string escalonada (linha de texto)
void ST7789_DrawStringScaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    uint16_t current_x = x;

    // Largura total de um caractere: FONT_WIDTH * scale + (1 pixel de espaço * scale)
    uint8_t char_spacing = 1 * scale; // Ajuste o espaçamento aqui se quiser mais ou menos
    uint16_t char_width_total = (FONT_WIDTH * scale) + char_spacing;

    while (*str) {
        ST7789_DrawCharScaled(current_x, y, *str, fg_color, bg_color, scale);

        // Avança a posição X para o próximo caractere
        current_x += char_width_total;

        // Verifica se ultrapassou o limite da tela (240)
        if (current_x + (FONT_WIDTH * scale) >= 240) {
            break;
        }

        str++;
    }
}
