/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h> // Para 'srand' e 'rand'
#include <stdio.h>  // Para 'sprintf'
#include "my_font.h"
#include "st7789.h"
#include "keypad.h"
#include "joystick.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

#define LED_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_13
// BASE DA VELOCIDADE:

// As variaveis g_ball_dx e g_ball_dy (agora em 2, 1) devem ser alteradas , setadas para teste
#define BALL_BASE_DX 2
#define BALL_BASE_DY 1

// --- VARIÁVEIS DA RAQUETE/PADDLE ---
#define PADDLE_WIDTH 5        // Largura da raquete (Eixo X)
#define PADDLE_HEIGHT 60
#define PADDLE_MOVE_STEP 4
// Raquete começa 5 pixels longe da borda direita (240 - 5 - 8 = 227)
#define PADDLE_X_POS (DISPLAY_WIDTH - PADDLE_WIDTH - 5) // **DEVE SER 227**
// Posição Y do topo da raquete (Inicializada no centro)
volatile uint16_t g_paddle_y = (DISPLAY_HEIGHT / 2) - (PADDLE_HEIGHT / 2);

// Variáveis de controle de tempo e estado
volatile uint32_t g_start_time = 0;
volatile uint8_t g_program_state = 0; // 0: Rodando (Bolinha), 1: Esperando Input

// VARIAVEL DE CONTROLE DE ESCALA
volatile float g_ball_speed_scale = 1.0f;
volatile uint32_t g_buzzer_duration_counter = 0;

// Frequência de 500Hz para o tom (Buzzer Passivo)
#define BUZZER_PERIOD_COLLISION 1999 // Periodo para ~500Hz com Prescaler 71
#define BUZZER_DUTY_CYCLE (BUZZER_PERIOD_COLLISION / 2) // 50% de ciclo de trabalho
#define BUZZER_COLLISION_MS 50 // Duração do som (50ms)

//Definicao JoyStick
#define JOY_SW_Port GPIOA
#define JOY_SW_Pin  GPIO_PIN_2

// Variável global para rastrear o estado anterior do botão
volatile uint8_t g_sw_joy_pressed_prev = 0; // 0 = Solto, 1 = Pressionado

//configuracoes para a conta matematica
volatile uint8_t g_num1;
volatile uint8_t g_num2;
volatile char g_operador; // '*' para multiplicação, '/' para divisão
volatile int16_t g_resposta_correta;

// Variavel de controle de velocidade do joystick
volatile uint8_t g_joystick_state = 0; // 0: Centro, 1: Cima, 2: Baixo
volatile uint8_t state_2_substate = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);

/* USER CODE BEGIN PFP */

//VARIAVEL FUNDO TELA
// Variável Global (volátil) para rastrear o estado da cor UTILIZADA NO TIMER 3
volatile uint8_t g_color_state = 0; // 0: Vermelho, 1: Azul

//VARIAVEL BOLINHA
// Posição atual da bolinha (coordenadas do centro)
volatile uint16_t g_ball_x = 120; // Centro X (240/2)
volatile uint16_t g_ball_y = 160; // Centro Y (320/2)

// Direção do movimento (pixels por quadro - 20 FPS)
volatile int8_t g_ball_dx = -2; // Move 2 pixels no eixo X por interrupção
volatile int8_t g_ball_dy = 1; // Move 1 pixel no eixo Y por interrupção
char input_word[16];

extern uint8_t Joystick_GetState(void);
//gerar um valor aleatorio RANDOM
uint32_t GenerateStrongSeed(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Funções para controlar o PWM do BUZZER ----------------------------------
void Buzzer_Start(void)
{
    // 1. Configurar o TIM1 para a frequência audível (3kHz)
    TIM_OC_InitTypeDef sConfigOC = {0};

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = BUZZER_DUTY_CYCLE;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    // Configura o Período para a frequência do tom (3kHz)
    htim1.Init.Period = BUZZER_PERIOD_COLLISION;
    HAL_TIM_Base_Init(&htim1); // Aplica o novo período

    // Configura e inicia o Canal 3 (CH3) - PA10 esta conectado aqui
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
      Error_Handler();
    }

    // 2. Iniciar o PWM (Se for Complementar - CH3N - pode precisar do Start/Stop N)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

void Buzzer_Stop(void)
{
    // Parar o PWM e garantir silêncio
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

    // Restaurar o Periodo do TIM1 para o Backlight (do seu código MX_TIM1_Init: 999)
    htim1.Init.Period = 999;
    HAL_TIM_Base_Init(&htim1);

    // Restaurar o ciclo de trabalho do Backlight (Assumindo 100% como no seu código)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 999);

    // Reiniciar o PWM do Backlight
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

/**
  * @brief Gera uma nova pergunta de multiplicação ou divisão.
  * @retval None
  */
void GenerateNewMathQuestion(void) {
    uint8_t op_choice = rand() % 2; // 0: Multiplicação, 1: Divisão

    if (op_choice == 0) { // Multiplicação (Geração de números entre 2 e 12)
        g_operador = 'x';

        // Mantemos um range menor (2 a 12) para que o resultado máximo (12*12 = 144)
        // caiba confortavelmente nas suas variáveis g_num1/g_num2 (uint8_t, max 255).
        g_num1 = (rand() % 11) + 2; // Range [2, 12]
        g_num2 = (rand() % 11) + 2; // Range [2, 12]

        g_resposta_correta = (int16_t)g_num1 * g_num2;
    } else { // Divisão (Geração de divisão com resultado inteiro)
        g_operador = '/';

        // 1. Gerar Quociente (Resultado) e Divisor (Ambos entre 2 e 12)
        uint8_t quociente = (rand() % 11) + 2; // Range [2, 12]
        uint8_t divisor = (rand() % 11) + 2;   // Range [2, 12]

        // 2. Calcular o Dividendo (g_num1) para garantir resultado inteiro
        g_num1 = divisor * quociente; // Dividendo = Divisor * Quociente

        // 3. Checagem de Overflow e Recalculo (Proteção)
        // Se o g_num1 (dividendo) for maior que 255 (máximo para uint8_t),
        // a conta pode ter problemas com a variável global g_num1.
        if (g_num1 > 255) {
             // Se houver overflow, forçamos um range menor, voltando à multiplicação
             // É melhor do que travar o jogo.
             g_operador = 'x';
             g_num1 = (rand() % 5) + 2;  // Range [2, 6]
             g_num2 = (rand() % 5) + 2;  // Range [2, 6]
             g_resposta_correta = (int16_t)g_num1 * g_num2;
             return;
        }

        // 4. Atribuir os valores finais
        g_num2 = divisor;              // O divisor
        g_resposta_correta = quociente; // O resultado (quociente)
    }
}

/**
  * @brief Desenha ou apaga a raquete na tela.
  * @param color: Cor para preencher.
  * @retval None
  */
void DrawPaddle(uint16_t color)
{
    // Define o canto superior esquerdo (x0, y0)
    uint16_t x0 = PADDLE_X_POS;
    uint16_t y0 = g_paddle_y;

    // Define o canto inferior direito (x1, y1)
    // Subtraímos 1 da largura e altura para incluir o pixel inicial.
    uint16_t x1 = PADDLE_X_POS + PADDLE_WIDTH - 1;
    uint16_t y1 = g_paddle_y + PADDLE_HEIGHT - 1;

    // Chama a função FillRect para desenhar o retângulo
    // Nota: A função FillRect que você compartilhou já lida com o WriteCommand(0x2C) e SPI.
    ST7789_FillRect(x0, y0, x1, y1, color);
}

/**
  * @brief Atualiza a posição da raquete baseado no joystick.
  * @retval None
  */
void UpdatePaddlePosition(void) {
    // 1. Salva a posição Y atual (antiga)
    uint16_t y_old = g_paddle_y;
	#define CLEANUP_MARGIN 2
    // 2. Lê o estado do joystick
    g_joystick_state = Joystick_GetState();

    // 3. Calcula a Nova Posição (g_paddle_y)
    if (g_joystick_state == 1) { // Cima
        // Usamos a subtração segura para evitar underflow (se g_paddle_y for uint16_t)
        if (g_paddle_y > PADDLE_MOVE_STEP) {
             g_paddle_y -= PADDLE_MOVE_STEP;
        } else {
             g_paddle_y = 0; // Se o movimento for maior que a posição atual, vai para 0
        }
    } else if (g_joystick_state == 2) { // Baixo
        g_paddle_y += PADDLE_MOVE_STEP;
    }

    // 4. Limita a raquete dentro dos limites (CLIPPING ROBUSTO)
    if (g_paddle_y < 0) {
        g_paddle_y = 0;
    }
    if (g_paddle_y > DISPLAY_HEIGHT - PADDLE_HEIGHT) {
        g_paddle_y = DISPLAY_HEIGHT - PADDLE_HEIGHT;
    }

    // 5. Desenho Otimizado: SOMENTE se a posição MUDOU
    if (g_paddle_y != y_old) {

        // --- APAGAR APENAS A DIFERENÇA (OTIMIZAÇÃO) ---

        // Se moveu para CIMA (y_old > g_paddle_y)
    	if (g_paddle_y < y_old) {

    	    // Área a ser apagada: A base que sobrou na posição antiga

    	    // 1. Ajuste X: Começa a apagar um pouco antes da raquete
    	    uint16_t x0 = PADDLE_X_POS - CLEANUP_MARGIN;

    	    // 2. Ajuste Y: O rastro na parte de baixo
    	    uint16_t y0 = y_old + PADDLE_HEIGHT - PADDLE_MOVE_STEP;
    	    uint16_t y1 = y_old + PADDLE_HEIGHT - 1;

    	    // 3. Ajuste X: Termina de apagar um pouco depois da raquete
    	    uint16_t x1 = PADDLE_X_POS + PADDLE_WIDTH - 1 + CLEANUP_MARGIN;

    	    // Apaga o rastro
    	    ST7789_FillRect(x0, y0, x1, y1, ST7789_COLOR_BLACK);
    	}

    	// Se moveu para BAIXO (g_paddle_y > y_old)
    	else if (g_paddle_y > y_old) {

    	    // Área a ser apagada: O topo que ficou para trás na posição antiga

    	    // 1. Ajuste X: Começa a apagar um pouco antes da raquete
    	    uint16_t x0 = PADDLE_X_POS - CLEANUP_MARGIN;

    	    // 2. Ajuste Y: O rastro na parte de cima
    	    uint16_t y0 = y_old;
    	    uint16_t y1 = y_old + PADDLE_MOVE_STEP - 1;

    	    // 3. Ajuste X: Termina de apagar um pouco depois da raquete
    	    uint16_t x1 = PADDLE_X_POS + PADDLE_WIDTH - 1 + CLEANUP_MARGIN;

    	    // Apaga o rastro
    	    ST7789_FillRect(x0, y0, x1, y1, ST7789_COLOR_BLACK);
    	}

        // --- DESENHAR O NOVO RETÂNGULO ---
        // Desenha o RETÂNGULO COMPLETO na nova posição (g_paddle_y)
        DrawPaddle(ST7789_COLOR_WHITE);
    }
    // Se a posição não mudou, não faz nada.
}

void GameOver(void){

      ST7789_DrawStringScaled(20, 100, "GAME OVER", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 4);
      // O loop permanece aqui até um reset (ou lógica de reinício, se você adicionar)
      HAL_Delay(100); // Pequeno delay para evitar que o MCU gire ocioso muito rápido
}

void new_game(void){
	  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
	  ST7789_DrawStringScaled(40, 100, "NEW GAME", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 4);
	  HAL_Delay(150);
	  cont_regressiva();
	  //Rotoma o jogo
	  // 2. REINICIALIZAÇÃO DA BOLINHA NO CENTRO
	  g_ball_x = DISPLAY_WIDTH / 2; // Centro Horizontal (320)
	  g_ball_y = DISPLAY_HEIGHT / 2; // Centro Vertical (240)

	  // Se BALL_BASE_DX for definido como 2, aqui será -2
	  g_ball_dx = -2; // Move 2 pixels no eixo X por interrupção
	  g_ball_dy = 1; // Move 1 pixel no eixo Y por interrupção)

	  g_program_state = 0; // Volta para o jogo
	  g_start_time = HAL_GetTick();
	  // A bolinha será desenhada na primeira interrupção do TIM2, mas a raquete deve ser imediata
	  DrawPaddle(ST7789_COLOR_WHITE); // Desenha a raquete na posição atual
	  ST7789_DrawBall(g_ball_x, g_ball_y, ST7789_COLOR_WHITE); // Garante que a bolinha apareça antes da interrupção
	  HAL_TIM_Base_Start_IT(&htim2); // RETOMA A ANIMAÇÃO
}

void cont_regressiva(void){
		// X = 108: Centralizado
		const uint16_t X_CENTERED = 148;
		const uint16_t Y_POSITION = 100; // Mantendo a posição vertical
		const uint8_t SCALE = 4;
	  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
	  ST7789_DrawStringScaled(X_CENTERED, Y_POSITION, "3", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);
	  HAL_Delay(150);
	  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
	  ST7789_DrawStringScaled(X_CENTERED, Y_POSITION, "2", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);
	  HAL_Delay(150);
	  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
	  ST7789_DrawStringScaled(X_CENTERED, Y_POSITION, "1", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, SCALE);
	  HAL_Delay(150);
	  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
}
/**
  * @brief Gerar um valor aleatri oatraves do ruido do joystick
  * @retval None
  */
uint32_t GenerateStrongSeed(void) {
    // Fatores variáveis para iteração:
    // 1. Contador de ticks atual (Alto valor após a introdução de 5 segundos)
    uint32_t current_tick = HAL_GetTick();

    // 2. Ruído do Joystick (ADC)
    uint32_t adc_reading = 0;

    // Lista de iteração: Iteramos sobre o número de conversões do ADC.
    const int NUM_ITERATIONS = 32;

    HAL_ADC_Start(&hadc1);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            // Combina o ADC com o tick e o índice do loop (i)
            adc_reading ^= HAL_ADC_GetValue(&hadc1) + current_tick + i;
        }
        // Usar um delay muito pequeno aqui aumenta a variância do ADC
        HAL_Delay(1);
    }

    HAL_ADC_Stop(&hadc1);

    // O valor final da semente é a combinação dos fatores iterados.
    return adc_reading ^ current_tick;
}
/**
  * @brief A função chamada pela HAL quando um Timer expira
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    // Verifica se a interrupção é do Timer 2
	if (htim->Instance == TIM2)
	    {
	    	// Executa apenas se estiver no estado de jogo (Rodando)
	    	if (g_program_state != 0) {
	    	    return; // Se não estiver rodando, apenas retorna
	    	}

	    	//Salvar a posição ANTIGA
			uint16_t x_old = g_ball_x;
			uint16_t y_old = g_ball_y;

	    	uint8_t collision_detected = 0;

	        // 1. Limpar (apagar) a bolinha na posição antiga:
	        //ST7789_DrawBall(g_ball_x, g_ball_y, ST7789_COLOR_BLACK);

	        // 2. CÁLCULO DE COLISÃO E BORDAS

	        // Verifica colisão nas laterais ESQUERDA (Eixo X: 0)
	        // NOTA: A colisão na lateral direita será a raquete ou o Game Over
	        if (g_ball_x + g_ball_dx <= BALL_RADIUS) {
	            g_ball_dx = -g_ball_dx;
	            collision_detected = 1;
			}
	        // Verifica colisão no topo/base (Eixo Y: 0 a 319)
			if (g_ball_y + g_ball_dy >= DISPLAY_HEIGHT - 1 - BALL_RADIUS || g_ball_y + g_ball_dy <= BALL_RADIUS) {
				g_ball_dy = -g_ball_dy;
				collision_detected = 1;
			}

			// --- COLISÃO COM A RAQUETE (LADO DIREITO) ---
	        // 1. Previsão da Próxima Posição da Borda Direita da Bolinha
	        uint16_t next_ball_right_x = g_ball_x + g_ball_dx + BALL_RADIUS;
	        uint16_t ball_y_top = g_ball_y - BALL_RADIUS;
	        uint16_t ball_y_bottom = g_ball_y + BALL_RADIUS;

	        // 2. Verifica se a bolinha está indo para a direita e cruzou a posição X da Raquete
	        if (g_ball_dx > 0 && next_ball_right_x >= PADDLE_X_POS) {

	            // 3. Verifica se a bolinha está dentro da altura (Y) da Raquete
	            if (ball_y_bottom >= g_paddle_y && ball_y_top <= g_paddle_y + PADDLE_HEIGHT) {
	                // COLISÃO COM A RAQUETE DETECTADA!
	            	g_ball_x = PADDLE_X_POS - BALL_RADIUS;
	                g_ball_dx = -g_ball_dx; // Inverte o movimento X
	                collision_detected = 1;

	                // Move a bolinha para evitar que ela "grude" na raquete
	                //g_ball_x = PADDLE_X_POS - BALL_RADIUS - 1;

	            } else {
	                // FIM DE JOGO: Bolinha passou pela direita
	                HAL_TIM_Base_Stop_IT(&htim2); // PÁRA A ANIMAÇÃO
	                // Apaga a bolinha antes de mudar o estado
	                ST7789_DrawBall(g_ball_x, g_ball_y, ST7789_COLOR_BLACK);
	                DrawPaddle(ST7789_COLOR_BLACK); // Apaga a raquete
	                g_program_state = 1; // Transiciona para o estado Game Over
	                return; // Sai da ISR
	            }
	        }
		if (collision_detected) {
				// Se o buzzer já estiver tocando, ignora (ou reinicia o tempo)
				if (g_buzzer_duration_counter == 0) {
				   g_buzzer_duration_counter = HAL_GetTick() + BUZZER_COLLISION_MS;
				   Buzzer_Start();
				}
		}
		// --- PASSO 2: Mover a Bolinha para a nova posição ---
		    // Fazemos o movimento SÓ DEPOIS de calcular o Bounding Box
		    uint16_t x_future = x_old + g_ball_dx;
		    uint16_t y_future = y_old + g_ball_dy;

		    // 3. Apagamento da Área Abrangente
		    // Calcula a caixa que cobre (x_old, y_old) e (x_future, y_future)

		    // Ponto superior esquerdo (mínimo X e mínimo Y)
		    uint16_t x_min = (x_old < x_future) ? x_old : x_future;
		    uint16_t y_min = (y_old < y_future) ? y_old : y_future;

		    // Ponto inferior direito (máximo X e máximo Y)
		    uint16_t x_max = (x_old > x_future) ? x_old : x_future;
		    uint16_t y_max = (y_old > y_future) ? y_old : y_future;

		    // Ajusta a caixa para incluir o Raio (BALL_RADIUS) e a margem de segurança (ex: +1 pixel)
		    // O apagamento deve ir de (x_min - R - 1) até (x_max + R + 1)

		    uint16_t x0_erase = x_min - BALL_RADIUS - 2;

		    uint16_t y0_erase = y_min - BALL_RADIUS - 2;

		    uint16_t x1_erase = x_max + BALL_RADIUS + 2;

		    uint16_t y1_erase = y_max + BALL_RADIUS + 2;

		    // Garante que não saímos dos limites da tela (0 a 319 / 0 a 239)
		    if (x0_erase < 0) x0_erase = 0;
		    if (y0_erase < 0) y0_erase = 0;
		    if (x1_erase >= DISPLAY_WIDTH) x1_erase = DISPLAY_WIDTH - 1;
		    if (y1_erase >= DISPLAY_HEIGHT) y1_erase = DISPLAY_HEIGHT - 1;

		    // Apaga a área completa com a cor de fundo (ST7789_COLOR_BLACK)
		    ST7789_FillRect(x0_erase, y0_erase, x1_erase, y1_erase, ST7789_COLOR_BLACK);

		    // 4. Mover a Bolinha para a nova posição e atualizar a global
		    g_ball_x = x_future;
		    g_ball_y = y_future;

		    // 5. Desenhar a bolinha na nova posição:
		    ST7789_DrawBall(g_ball_x, g_ball_y, ST7789_COLOR_WHITE);

    }

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  Joystick_Init(&hadc1);

  /* USER CODE BEGIN 2 */

  	// 1. Ligar o Backlight (Brilho Máximo - 100%)
  	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 999); // Period=999

  	// 2. Inicializar o Display ST7789
  	ST7789_Init();
  	// 3. Ligar a Tela em uma cor escolhida
  	ST7789_FillColor(ST7789_COLOR_BLACK);

  	// --- DEFINIÇÕES PARA O TEXTO ---
  	uint16_t TEXT_X = 10;
  	uint16_t TEXT_Y_LINE1 = 10;
  	uint16_t TEXT_Y_LINE2 = 45;
 	uint16_t TEXT_Y_LINE3 = 80;
  	uint8_t scale_factor = 3;  //definir a escala que será escrito

  	// LINHA 1
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE1, "OLA", ST7789_COLOR_WHITE, ST7789_COLOR_WHITE, scale_factor);
  	// LINHA 2
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE2, "BEM VINDO AO", ST7789_COLOR_WHITE, ST7789_COLOR_WHITE, scale_factor);
  	// LINHA 3
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE3, "PONG", ST7789_COLOR_WHITE, ST7789_COLOR_WHITE, 4);
  	HAL_Delay(1500);
  	srand(GenerateStrongSeed());

  	// 1. Apagar a LINHA 1: Redesenha "OLA" usando a cor BRANCA (cor de fundo)
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE1, "OLA", ST7789_COLOR_BLACK, ST7789_COLOR_WHITE, scale_factor);

  	// 2. Apagar a LINHA 2: Redesenha "FIN" usando a cor BRANCA (cor de fundo)
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE2, "BEM VINDO AO", ST7789_COLOR_BLACK, ST7789_COLOR_WHITE, scale_factor);

  	// 2. Apagar a LINHA 3: Redesenha "" usando a cor BRANCA (cor de fundo)
  	ST7789_DrawStringScaled(TEXT_X, TEXT_Y_LINE3, "PONG", ST7789_COLOR_BLACK, ST7789_COLOR_WHITE, 4);
  	ST7789_FillColor(ST7789_COLOR_BLACK);
  	HAL_Delay(1000);
  	cont_regressiva();
  	g_program_state = 0; // Inicia no estado de "Rodando"
  	// 1. Iniciar Timer da Bolinha e Estado
  	HAL_TIM_Base_Start_IT(&htim2); // Timer 50ms (animação)

  	// 2. Iniciar a contagem de 10 segundos
  	g_start_time = HAL_GetTick();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	while (1)
  	  {
  		  //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  		  HAL_Delay(50); // Mantenha um delay baixo para boa resposta do joystick
  		  //Buzzer Stop
  		  if (g_buzzer_duration_counter > 0 && HAL_GetTick() >= g_buzzer_duration_counter) {
  		            Buzzer_Stop();
  		            g_buzzer_duration_counter = 0;
  		  }
  		  // --- FLUXO DE ESTADOS ---

  		if (g_program_state == 0) {
  		          // Estado 0: Rodando Normal (JOGO)
  		          UpdatePaddlePosition();
  		}

  		else if (g_program_state == 1) {
  		          // Estado 2: DESAFIO MATEMÁTICO (Processamento de Input)

  		          if (state_2_substate == 0) {
  		              // SUBESTADO 0: CONFIGURAÇÃO/PREPARAR TELA
  		              GenerateNewMathQuestion();

  		              // Desenha o prompt
  		              char question_str[30];
  		              sprintf(question_str, "%d %c %d = ?", g_num1, g_operador, g_num2);

  		              ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
  		              HAL_Delay(100);

  		              ST7789_DrawStringScaled(10, 50, "DESAFIO", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 3);
  		              ST7789_DrawStringScaled(10, 100, question_str, ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 2);

  		              // Limpar o buffer de input (importante para nova entrada)
  		              memset(input_word, 0, sizeof(input_word));

  		              state_2_substate = 1; // Próximo subestado: Esperar Input
  		          }

  		          else if (state_2_substate == 1) {
  		              // SUBESTADO 1: CHAMA A FUNÇÃO BLOQUEANTE E RETORNA AQUI COM A RESPOSTA
  		              // Se KEYPAD_GetWord é BLOQUEANTE, ele travará aqui até 'D' ser pressionado.
  		              KEYPAD_GetWord(input_word, 16);

  		              // Se a função KEYPAD_GetWord() bloqueia o código e só retorna quando
  		              // o usuário finaliza, o código segue diretamente para o Subestado 2.
  		              state_2_substate = 2;
  		          }

  		          else if (state_2_substate == 2) {
  		              // SUBESTADO 2: VALIDAÇÃO E TRANSIÇÃO
  		              int16_t user_answer = atoi(input_word);

  		              if (user_answer == g_resposta_correta) {
  		                  // CORRETO: Retorna ao jogo
  		                  ST7789_DrawStringScaled(10, 180, "CORRETO!", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 3);
  		                  HAL_Delay(1500);

  		                  // Retorna ao Jogo
  		                  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela para o jogo
  		                  HAL_Delay(150);

  		                // 2. REINICIALIZAÇÃO DA BOLINHA NO CENTRO
						  g_ball_x = DISPLAY_WIDTH / 2; // Centro Horizontal (320)
						  g_ball_y = DISPLAY_HEIGHT / 2; // Centro Vertical (240)

						  // Se BALL_BASE_DX for definido como 2, aqui será -2
						  g_ball_dx = -2; // Move 2 pixels no eixo X por interrupção
						  g_ball_dy = 1; // Move 1 pixel no eixo Y por interrupção)

  		                  g_program_state = 0; // Volta para o jogo
  		                  g_start_time = HAL_GetTick();
  		                  // A bolinha será desenhada na primeira interrupção do TIM2, mas a raquete deve ser imediata
						  DrawPaddle(ST7789_COLOR_WHITE); // Desenha a raquete na posição atual
						  ST7789_DrawBall(g_ball_x, g_ball_y, ST7789_COLOR_WHITE); // Garante que a bolinha apareça antes da interrupção
		                  HAL_TIM_Base_Start_IT(&htim2); // RETOMA A ANIMAÇÃO

  		              } else {
  		                  // ERRADO: Game Over
  		                  ST7789_DrawStringScaled(10, 180, "ERRADO!", ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 3);
  		                  char correct_str[30];
  		                  sprintf(correct_str, "Resp correta: %d", g_resposta_correta);
  		                  ST7789_DrawStringScaled(10, 220, correct_str, ST7789_COLOR_WHITE, ST7789_COLOR_BLACK, 2);
  		                  HAL_Delay(3000);
  						  ST7789_FillColor(ST7789_COLOR_BLACK); // Limpa a tela
  		                  g_program_state = 2; // Transiciona para Game Over
  		              }
  		              state_2_substate = 0; // Reseta o subestado para a próxima rodada

  		          }
  		      }

  		      else if (g_program_state == 2) {
  		          // Estado 2: GAME OVER
  		    	  GameOver();

  		    	// Lê o estado do pino (assumindo que LOW = Pressionado)
  		    	    GPIO_PinState joy_state = HAL_GPIO_ReadPin(JOY_SW_Port, JOY_SW_Pin);

  		    	    if (joy_state == GPIO_PIN_RESET) { // RESET ou LOW significa Pressionado (se configurado com Pull-up)
  		    	         // Inicia um novo jogo.
  		    	         new_game();
  		    	    }
  		      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 359;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 35999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|teste_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, R1_Pin|R2_Pin|R4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_JOY_Pin */
  GPIO_InitStruct.Pin = SW_JOY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SW_JOY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : teste_Pin R3_Pin */
  GPIO_InitStruct.Pin = teste_Pin|R3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : C1_Pin C2_Pin C3_Pin C4_Pin */
  GPIO_InitStruct.Pin = C1_Pin|C2_Pin|C3_Pin|C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : R1_Pin R2_Pin R4_Pin */
  GPIO_InitStruct.Pin = R1_Pin|R2_Pin|R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
