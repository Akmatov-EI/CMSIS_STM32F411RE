#include "stm32f4xx.h"
#include <string.h>

#define BUFFER_SIZE 20

volatile uint32_t ticks = 0;

void SystemClock_Config(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // Включаем тактирование для TIM2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Включаем тактирование для GPIOA

    // Настраиваем PA5 на альтернативную функцию
    GPIOA->MODER |= (2 << (5 * 2));   // AF (mode=10)
    GPIOA->OSPEEDR |= (3 << (5 * 2)); // High speed
    GPIOA->AFR[0] |= (1 << (5 * 4));  // TIM2_CH1

    // Настройка TIM2 для ШИМ
    TIM2->PSC = 1600 - 1; // Предделитель (16 МГц / 1600 = 10 кГц)
    TIM2->ARR = 100 - 1;  // Период PWM (10 кГц / 100 = 100 Гц)
    TIM2->CCR1 = 50;      // Ширина импульса (50% DUTY)

    TIM2->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos); // PWM mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;           // Разрешить предобработка
    TIM2->CCER |= TIM_CCER_CC1E;              // Включить канал 1
    TIM2->CR1 |= TIM_CR1_CEN;                 // Запуск таймера
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {                            // Проверяем флаг обновления
        TIM2->SR &= ~TIM_SR_UIF; // Сбрасываем флаг
        ticks++;                 // Увеличиваем счетчик
    }
}

void timer_init(void)
{
    // Включаем тактирование TIM2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    // Устанавливаем значение авто-перезагрузки для 1 мс при 84 МГц
    TIM2->PSC = 8399;           // Предделитель = 8399 + 1
    TIM2->ARR = 99;             // Авто-перезагрузка = 100, т.е., 1 мс
    TIM2->DIER |= TIM_DIER_UIE; // Разрешаем прерывание по обновлению
    TIM2->CR1 |= TIM_CR1_CEN;   // Запускаем таймер
    NVIC_EnableIRQ(TIM2_IRQn);  // Разрешаем прерывание в NVIC
}

uint32_t get_ticks(void)
{
    return ticks;
}

// Функция инициализации UART
void UART_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Тактирование порта A

    // Настройка PA2 (TX) и PA3 (RX) как альтернативные функции
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |= (2U << (2 * 2));
    GPIOA->AFR[0] |= (7 << (2 * 4));

    GPIOA->MODER &= ~(3U << (3 * 2));
    GPIOA->MODER |= (2U << (3 * 2));
    GPIOA->AFR[0] |= (7 << (3 * 4));

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Тактирование USART2

    // Настройка USART2
    USART2->BRR = 16000000 / 9600;
    USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

// Отправка символа через UART
void UART_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE))
        ;
    USART2->DR = c;
}

// Отправка строки через UART
void UART_SendString(const char *str)
{
    while (*str)
    {
        UART_SendChar(*str++);
    }
}

// Получение символа через UART
char UART_ReceiveChar(void)
{
    while (!(USART2->SR & USART_SR_RXNE))
        ;
    return USART2->DR;
}
uint32_t timer = 0;

// Основная функция
int main(void)
{
    UART_Init(); // Инициализация UART
    timer_init();
    UART_SendString("Start Control PWM Module\n");
    UART_SendString("Send parameters for PWM signal:\n");
    SystemClock_Config();
    while (1)
    {
       // char receivedChar = UART_ReceiveChar();
        if (get_ticks() >= 500)
        {
            // Работать с 1 секундой
            UART_SendString("hello\n");
            ticks = 0; // Сбрасываем счетчик
        }
    }
}
