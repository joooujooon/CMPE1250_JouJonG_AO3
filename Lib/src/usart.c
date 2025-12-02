///////////////////////////////////////////////////////////////////////// 
//
//  USART LIBRARY
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   usart.c
//  Version History
//    ICA05 - Basic USART Library
//    ICA06 - Advanced USART Library (strings, cursor, screen control)
//
/////////////////////////////////////////////////////////////////////////

#include "stm32g031xx.h"
#include "usart.h"
#include <stdio.h>   // For sprintf formatting
#define SEGGER_RTT_DISABLE
#define DISABLE_RTT


// =====================================================================
//  ICA05 BASIC INITIALIZATION + BLOCKING/NONBLOCKING BYTE IO
// =====================================================================

void _USART_Init_USART2(uint32_t sysclk, uint32_t baud)
{
    // Enable GPIOA
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // PA2 = TX (AF1), PA3 = RX (AF1)
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2))); // Clear mode
    GPIOA->MODER |=  (2U << (2 * 2)) | (2U << (3 * 2));   // Alternate function

    GPIOA->AFR[0] &= ~((0xF << (4 * 2)) | (0xF << (4 * 3)));
    GPIOA->AFR[0] |=  ((1U << (4 * 2)) | (1U << (4 * 3))); // AF1 for USART2

    // Enable USART2 peripheral clock
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    // Disable USART before config
    USART2->CR1 &= ~USART_CR1_UE;

    // Compute BRR
    USART2->BRR = (uint32_t)(sysclk / baud);

    // Enable transmitter + receiver
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enable USART2
    USART2->CR1 |= USART_CR1_UE;
}


// ---------------------------------------------------------------------
// Send a single byte (blocking)
// ---------------------------------------------------------------------
void _USART_TxByte(USART_TypeDef *pUSART, char data)
{
    while (!(pUSART->ISR & USART_ISR_TXE_TXFNF))
        ;
    pUSART->TDR = data;
}


// ---------------------------------------------------------------------
// Non-blocking receive of a byte
// ---------------------------------------------------------------------
int _USART_RxByte(USART_TypeDef *pUSART, char *pData)
{
    if (pUSART->ISR & USART_ISR_RXNE_RXFNE)
    {
        *pData = pUSART->RDR;
        return 1;
    }
    return 0;
}


// =====================================================================
//  ICA06 ADVANCED FUNCTIONS
// =====================================================================

// ---------------------------------------------------------------------
// Send a NULL-terminated string
// ---------------------------------------------------------------------
void _USART_TxString(USART_TypeDef *pUSART, const char *s)
{
    if (!s) return;

    while (*s != '\0')
    {
        _USART_TxByte(pUSART, *s);
        s++;
    }
}


// ---------------------------------------------------------------------
// Move cursor to (col,row) using ESC[row;colH
// ---------------------------------------------------------------------
void _USART_GotoXY(USART_TypeDef *pUSART, int col, int row)
{
    char escSeq[20];
    sprintf(escSeq, "\x1b[%d;%dH", row, col);
    _USART_TxString(pUSART, escSeq);
}


// ---------------------------------------------------------------------
// Move cursor then print string
// ---------------------------------------------------------------------
void _USART_TxStringXY(USART_TypeDef *pUSART, int col, int row, const char *s)
{
    _USART_GotoXY(pUSART, col, row);
    _USART_TxString(pUSART, s);
}


// ---------------------------------------------------------------------
// Clear terminal screen + home cursor
// ---------------------------------------------------------------------
void _USART_ClearScreen(USART_TypeDef *pUSART)
{
    _USART_TxString(pUSART, "\x1b[2J"); // Clear screen
    _USART_TxString(pUSART, "\x1b[H");  // Cursor home
}


// ---------------------------------------------------------------------
// BLOCKING receive of one byte
// ---------------------------------------------------------------------
unsigned char _USART_RxByteB(USART_TypeDef *pUSART)
{
    while (!(pUSART->ISR & USART_ISR_RXNE_RXFNE))
        ;
    return pUSART->RDR;
}


// ---------------------------------------------------------------------
// Receive full string 
// ---------------------------------------------------------------------
int _USART_RxString(USART_TypeDef *pUSART,
                    unsigned char *pTargetBuffer,
                    unsigned short iBufferLength,
                    _USART_RX_ENFORCE EnforceType)
{
    if (!pTargetBuffer || iBufferLength < 1)
        return 0;

    int index = 0;

    while (index < iBufferLength - 1)
    {
        unsigned char c = _USART_RxByteB(pUSART);

        // ---------------------------------------------------------
        // BACKSPACE
        // ---------------------------------------------------------
        if (c == '\b' || c == 127)
        {
            if (index > 0)
            {
                index--;
                pTargetBuffer[index] = '\0';
                _USART_TxString(pUSART, "\b \b"); // erase visual char
            }
            continue;
        }

        // ---------------------------------------------------------
        // ENTER KEY
        // ---------------------------------------------------------
        if (c == '\r' || c == '\n')
        {
            pTargetBuffer[index] = '\0';
            return index;
        }

        // ---------------------------------------------------------
        // ENFORCEMENT RULES
        // ---------------------------------------------------------
        int valid = 0;

        switch (EnforceType)
        {
            case _USART_RX_ENFORCE_ANY:
                if (c >= ' ' && c <= '~')
                    valid = 1;
                break;

            case _USART_RX_ENFORCE_DIGIT:
                if (c >= '0' && c <= '9')
                    valid = 1;
                break;

            case _USART_RX_ENFORCE_HEX:
                if ((c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'f') ||
                    (c >= 'A' && c <= 'F'))
                    valid = 1;
                break;
        }

        // If valid, echo and store
        if (valid)
        {
            pTargetBuffer[index] = c;
            index++;
            _USART_TxByte(pUSART, c); // Echo it
        }
        // Otherwise ignore the character
    }

    pTargetBuffer[index] = '\0';
    return index;
}
int __io_putchar(int ch)
{
    _USART_TxByte(USART2, ch);
    return ch;
}

int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
        _USART_TxByte(USART2, ptr[i]);
    return len;
}
