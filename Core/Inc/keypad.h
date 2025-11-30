/* Core/Inc/keypad.h */

#ifndef __KEYPAD_H
#define __KEYPAD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h" // Inclui definições básicas (HAL, uint8_t, etc.)

// --- Definições de Estado Global para a Lógica de Interrupção ---
// Flag para controlar se o teclado está esperando input (1) ou não (0).
volatile extern uint8_t g_keypad_active;
// Buffer onde os caracteres digitados serão armazenados.
volatile extern char g_keypad_buffer[16];
// Índice para rastrear onde o próximo caractere deve ser salvo.
volatile extern uint8_t g_buffer_index;
// Mapeamento dos Caracteres (Externo)
extern const char KEYPAD_KEYS[4][4];


// --- Protótipos das Funções ---

// Função principal de leitura do teclado (faz a varredura)
char KEYPAD_ReadKey(void);

// Função para iniciar o modo de espera de entrada do teclado (chamada pelo "jogo")
void KEYPAD_StartInput(void);
void KEYPAD_GetWord(char *buffer, uint8_t max_len);
// Função de Inicialização (Opcional, mas útil para definir o estado inicial dos pinos)
// void KEYPAD_Init(void); // Você pode pular isso se MX_GPIO_Init for suficiente


#ifdef __cplusplus
}
#endif

#endif /* __KEYPAD_H */
