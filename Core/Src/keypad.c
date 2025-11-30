/* Core/Src/keypad.c */

#include "keypad.h"
#include "my_font.h"
#include "main.h"
#include "st7789.h"
// 1. Mapeamento dos Caracteres (Baseado na foto)
// Se a fiação R1-R4 e C1-C4 for esta:
const char KEYPAD_KEYS[4][4] = {
    // C1    C2    C3    C4
    {'1', '4', '7', '*'}, // R1
    {'2', '5', '8', '0'}, // R2
    {'3', '6', '9', '#'}, // R3
    {'A', 'B', 'C', 'D'}  // R4 (ou C é a D?) - Ajuste conforme o layout do seu teclado!
};

// 2. Definição dos Pinos (Adapte com seus pinos reais)
// (Assumindo que você definiu os pinos acima em main.h ou usando HAL_GPIO_WritePin)
#define R1_PORT GPIOA
#define R1_PIN  GPIO_PIN_5

volatile uint8_t g_keypad_active = 0; // Começa DESATIVADO
volatile char g_keypad_buffer[16];
volatile uint8_t g_buffer_index = 0;

// (Assumindo que htim2 é o timer da sua bolinha)
extern TIM_HandleTypeDef htim2;
extern void ST7789_DrawStringScaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale);
// Pinos de Linha (Output)
// R1: A8, R2: A9, R3: B14, R4: A11
const uint16_t ROW_PINS[4] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_14, GPIO_PIN_11};
const GPIO_TypeDef* ROW_PORTS[4] = {GPIOA, GPIOA, GPIOB, GPIOA}; // Note o GPIOB no índice 2

// Pinos de Coluna (Input/EXTI)
// C1: B12, C2: B13, C3: B7, C4: B8
const uint16_t COL_PINS[4] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_7, GPIO_PIN_8};
const GPIO_TypeDef* COL_PORTS[4] = {GPIOB, GPIOB, GPIOB, GPIOB}; // Todos os pinos de Coluna estão no GPIOB (Ótimo!)

// 3. Função Principal de Leitura
char KEYPAD_ReadKey(void) {
    char key_pressed = 0;
    int row, col;

    // Loop de Varredura (ROW)
    for (row = 0; row < 4; row++) {

        // Coloca a Linha 'row' em LOW (varredura ativa)
        HAL_GPIO_WritePin((GPIO_TypeDef *)ROW_PORTS[row], ROW_PINS[row], GPIO_PIN_RESET);

        // Loop de Leitura (COLUMN)
        for (col = 0; col < 4; col++) {

            // Lê o pino da coluna: se LOW, a tecla naquela (row, col) foi pressionada
            if (HAL_GPIO_ReadPin((GPIO_TypeDef *)COL_PORTS[col], COL_PINS[col]) == GPIO_PIN_RESET) {

                // Tecla encontrada! Armazena o valor
                key_pressed = KEYPAD_KEYS[row][col];

                // Espera um pouco para debouncing (ajuste este valor)
                HAL_Delay(100);

                // Coloca a Linha de volta em HIGH (reseta a varredura)
                HAL_GPIO_WritePin((GPIO_TypeDef *)ROW_PORTS[row], ROW_PINS[row], GPIO_PIN_SET);

                // Espera o debounce terminar para evitar repetição (polling)
                while (HAL_GPIO_ReadPin((GPIO_TypeDef *)COL_PORTS[col], COL_PINS[col]) == GPIO_PIN_RESET);

                return key_pressed;
            }
        }

        // Coloca a Linha de volta em HIGH (prepara para a próxima linha)
        HAL_GPIO_WritePin((GPIO_TypeDef *)ROW_PORTS[row], ROW_PINS[row], GPIO_PIN_SET);
    }
    return 0; // Nenhum botão pressionado
}
void KEYPAD_StartInput(void) {
        // 1. Limpa o buffer e reseta o índice
        g_buffer_index = 0;
        g_keypad_buffer[0] = '\0';

        // 2. Define o flag (Ativa a interrupção do teclado, desativa a bolinha)
        g_keypad_active = 1;

        // 3. Para o Timer da bolinha (Interrompe a animação)
        HAL_TIM_Base_Stop_IT(&htim2);

        // 4. Desenha uma indicação na tela (Exemplo: Limpa e pede input)
        // Coordenadas dependem da sua orientação (assumindo 320x240 e escala 4x)
        //uint8_t scale = 4;
        ST7789_DrawStringScaled(10, 80, "ENTRADA:", ST7789_COLOR_WHITE, ST7789_COLOR_WHITE, 4);
        // Desenha uma linha vazia para a entrada
        ST7789_DrawStringScaled(10, 110, "___", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 4);
 }
// 4. Função de Entrada de Palavra (Bloqueante)
// Recebe o buffer onde a palavra será armazenada e o tamanho máximo
void KEYPAD_GetWord(char *buffer, uint8_t max_len) {

    uint8_t current_index = 0;
    char key = 0;

    // Assegura que o buffer esteja vazio no início
    buffer[0] = '\0';

    // 1. Prepara a Tela e Para o Jogo
    // Coordenadas para o display
    const uint16_t INPUT_X = 10;
    const uint16_t INPUT_Y_PROMPT = 130;
    const uint16_t INPUT_Y_TEXT = 150;
    const uint8_t SCALE = 2; // Escala do texto de entrada

    // Para o timer da bolinha (Interrompe a animação)
    //extern TIM_HandleTypeDef htim2;
    //HAL_TIM_Base_Stop_IT(&htim2);

    // Limpa a área de prompt e escreve o prompt inicial
    ST7789_FillRect(0, INPUT_Y_PROMPT, DISPLAY_WIDTH, 140, ST7789_COLOR_BLACK);
    ST7789_DrawStringScaled(INPUT_X, INPUT_Y_PROMPT, "DIGITE:", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);

    // Loop de espera pela entrada (BLOQUEANTE)
    while (1) {

        // --- 1. LÊ UMA TECLA ---
        key = KEYPAD_ReadKey(); // Chama a sua função de varredura

        // --- 2. PROCESSA A TECLA ---
        if (key != 0) {

            // Lógica de finalização: Tecla 'D'
            if (key == 'D') {
                buffer[current_index] = '\0'; // Termina a string

                // Mensagem de confirmação (opcional)
                //ST7789_DrawStringScaled(INPUT_X, 150, "INPUT FINALIZADO", ST7789_COLOR_BLUE, ST7789_COLOR_BLACK, 3);
                HAL_Delay(500); // Dá tempo para o usuário ver

                return; // Sai da função (o retorno do timer fica no main.c)
            }

            // Lógica de Backspace/Delete (Usando '*' como você sugeriu)
            else if (key == '*') {
                if (current_index > 0) {
                    current_index--;
                    buffer[current_index] = '\0'; // Remove o último caractere

                    // Limpa a linha de texto e redesenha o buffer
                    ST7789_FillRect(INPUT_X, INPUT_Y_TEXT, DISPLAY_WIDTH, INPUT_Y_TEXT + (10 * SCALE), ST7789_COLOR_BLACK);
                    ST7789_DrawStringScaled(INPUT_X, INPUT_Y_TEXT, buffer, ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);
                }
            }

            // Lógica de Armazenamento: Tecla normal
            else if (current_index < max_len - 1) { // max_len - 1 para o '\0'

                buffer[current_index++] = key;
                buffer[current_index] = '\0'; // Garante que é uma string válida

                // Exibe o caractere na tela (Desenha o buffer inteiro)
                ST7789_DrawStringScaled(INPUT_X, INPUT_Y_TEXT, buffer, ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);
            }
        }

        // Pequeno delay para evitar que a CPU fique 100% ocupada no loop (Polling)
        HAL_Delay(10);
    }
}
