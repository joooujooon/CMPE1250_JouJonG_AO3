#ifndef USART_H
#define USART_H

#include "stm32g031xx.h"
#include <stdint.h>

// ======================================================
// USART TERMINAL SIZE (ANSI)
// ======================================================
#define _USART_ROWS  24
#define _USART_COLS  80

// ======================================================
// FUNCTION PROTOTYPES
// ======================================================

// Initialize USART2
void _USART_Init_USART2(uint32_t sysclk, uint32_t baud);

// Transmit
void _USART_TxByte(USART_TypeDef *uart, char c);
void _USART_TxString(USART_TypeDef *uart, const char *str);

// Receive
char _USART_RxByteB(USART_TypeDef *uart);          // BLOCKING
uint8_t _USART_RxByte(USART_TypeDef *uart, char *c); // NON-BLOCKING

// Terminal control
void _USART_ClearScreen(USART_TypeDef *uart);
void _USART_SetCursor(USART_TypeDef *uart, uint8_t row, uint8_t col);
void _USART_TxStringXY(USART_TypeDef *uart, uint8_t col, uint8_t row, const char *str);

#endif
