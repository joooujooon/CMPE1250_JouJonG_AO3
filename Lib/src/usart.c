#include "usart.h"
#include <stdio.h>

// ======================================================
// INITIALIZE USART2
// ======================================================
void _USART_Init_USART2(uint32_t sysclk, uint32_t baud)
{
    // Enable clocks
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;

    // PA2 = TX, PA3 = RX (AF1)
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |=  (2U << (2 * 2));
    GPIOA->MODER &= ~(3U << (3 * 2));
    GPIOA->MODER |=  (2U << (3 * 2));

    GPIOA->AFR[0] &= ~(0xF << (2 * 4));
    GPIOA->AFR[0] |=  (1U << (2 * 4));
    GPIOA->AFR[0] &= ~(0xF << (3 * 4));
    GPIOA->AFR[0] |=  (1U << (3 * 4));

    // Disable USART
    USART2->CR1 &= ~USART_CR1_UE;

    // Baud rate
    USART2->BRR = sysclk / baud;

    // Enable TX, RX
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enable USART
    USART2->CR1 |= USART_CR1_UE;
}

// ======================================================
// TRANSMIT FUNCTIONS
// ======================================================
void _USART_TxByte(USART_TypeDef *uart, char c)
{
    while (!(uart->ISR & USART_ISR_TXE_TXFNF));
    uart->TDR = c;
}

void _USART_TxString(USART_TypeDef *uart, const char *str)
{
    while (*str)
        _USART_TxByte(uart, *str++);
}

// ======================================================
// RECEIVE FUNCTIONS
// ======================================================
char _USART_RxByteB(USART_TypeDef *uart)
{
    while (!(uart->ISR & USART_ISR_RXNE_RXFNE));
    return uart->RDR;
}

uint8_t _USART_RxByte(USART_TypeDef *uart, char *c)
{
    if (uart->ISR & USART_ISR_RXNE_RXFNE)
    {
        *c = uart->RDR;
        return 1;
    }
    return 0;
}

// ======================================================
// TERMINAL CONTROL (ANSI)
// ======================================================
void _USART_ClearScreen(USART_TypeDef *uart)
{
    _USART_TxString(uart, "\033[2J");
    _USART_TxString(uart, "\033[H");
}

void _USART_SetCursor(USART_TypeDef *uart, uint8_t row, uint8_t col)
{
    char buf[16];
    sprintf(buf, "\033[%d;%dH", row, col);
    _USART_TxString(uart, buf);
}

void _USART_TxStringXY(USART_TypeDef *uart, uint8_t col, uint8_t row, const char *str)
{
    _USART_SetCursor(uart, row, col);
    _USART_TxString(uart, str);
}
