/////////////////////////////////////////////////////////////////////////
//
//  USART LIBRARY 
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   usart.h
//  Version History
//    ICA05 - Basic USART Library
//    ICA06 - Advanced USART Library (strings, cursor, screen control)
//
/////////////////////////////////////////////////////////////////////////

#ifndef USART_LIB_H
#define USART_LIB_H

#include "stm32g031xx.h"
#include <stdint.h>
#include <stdio.h>      

// =====================================================================
// Terminal Geometry (fixed for PuTTY default 80×24)
// =====================================================================
#define _USART_ROWS         24
#define _USART_COLS         80

// =====================================================================
// Default receive buffer size
// =====================================================================
#define _USART_BUFFSIZE     64

// =====================================================================
// Enforcement options for RxString (ICA06 Part D)
// =====================================================================
typedef enum
{
    _USART_RX_ENFORCE_ANY,     // Allow any printable character
    _USART_RX_ENFORCE_DIGIT,   // Only allow 0–9
    _USART_RX_ENFORCE_HEX      // Allow 0–9, A–F, a–f
} _USART_RX_ENFORCE;


// =====================================================================
// BASIC ICA05 FUNCTIONS
// =====================================================================

/**
 * @brief Initialize USART2 on PA2/PA3 for TX/RX at given baud rate.
 */
void _USART_Init_USART2(uint32_t sysclk, uint32_t baud);

/**
 * @brief Send one character (blocking).
 */
void _USART_TxByte(USART_TypeDef *pUSART, char data);

/**
 * @brief Non-blocking receive of one character.
 * @return 1 if character read, 0 otherwise.
 */
int _USART_RxByte(USART_TypeDef *pUSART, char *pData);


// =====================================================================
// ICA06 ADVANCED FUNCTIONS — STRINGS, SCREEN, CURSOR, INPUT
// =====================================================================

/**
 * @brief Send a NULL-terminated string using repeated TxByte.
 */
void _USART_TxString(USART_TypeDef *pUSART, const char *s);

/**
 * @brief Move cursor to (col,row) using ANSI escape sequence ESC[row;colH.
 */
void _USART_GotoXY(USART_TypeDef *pUSART, int col, int row);

/**
 * @brief Move cursor then print a string.
 */
void _USART_TxStringXY(USART_TypeDef *pUSART, int col, int row, const char *s);

/**
 * @brief Clear the terminal screen and return cursor to home (1,1).
 */
void _USART_ClearScreen(USART_TypeDef *pUSART);

/**
 * @brief Blocking receive of one character (wait until RXNE).
 */
unsigned char _USART_RxByteB(USART_TypeDef *pUSART);

/**
 * @brief Receive a full editable string with backspace, enter, and enforcement.
 *
 * @param pUSART        USART instance (e.g., USART2)
 * @param pTargetBuffer Pointer to destination buffer
 * @param iBufferLength Max characters INCLUDING NULL terminator
 * @param EnforceType   Restricts valid input (ANY/DIGIT/HEX)
 *
 * @return Number of characters stored (not counting NULL)
 */
int _USART_RxString(USART_TypeDef *pUSART,
                    unsigned char *pTargetBuffer,
                    unsigned short iBufferLength,
                    _USART_RX_ENFORCE EnforceType);


#endif // USART_LIB_H
