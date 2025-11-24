/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************

File    : main.c - ICA06 
Author  : Jou Jon Galenzoga
*/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "usart.h"

// ================================================================
// SELECT WHICH PART TO RUN
// ================================================================
//#define RUN_PART_A
//#define RUN_PART_B
//#define RUN_PART_C
#define RUN_PART_D


int main(void)
{
    //-----------------------------------------------------------------
    // COMMON GPIO INITIALIZATION (used or not, safe for all parts)
    //-----------------------------------------------------------------
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOC);

    _GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);   // Part A & C optional LED
    _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_Output);   // Part B scope timing

    //-----------------------------------------------------------------
    // ========================= PART A ================================
    // Terminal formatting using clear screen + cursor control
    //-----------------------------------------------------------------
#ifdef RUN_PART_A

    // Part A: USART @ 38400 baud
    _USART_Init_USART2(SystemCoreClock, 38400);                 // Part A

    // Part A: Clear terminal
    _USART_ClearScreen(USART2);                                 // Part A

    // Part A: Print name at top-left
    _USART_TxStringXY(USART2, 1, 1, "Jou Jon Galenzoga");       // Part A

    // Part A: Print CMPE1250 bottom-right (80×24 terminal)
    int lastCol = _USART_COLS - 7;                              // Part A
    int lastRow = _USART_ROWS;                                  // Part A
    _USART_TxStringXY(USART2, lastCol, lastRow, "CMPE1250");    // Part A

    while (1)
    {
        // Optional LED blink
        // _GPIO_PinToggle(GPIOC, 6);
        // for (volatile int i=0; i<200000; i++);
    }

#endif
// ========================= END PART A ===============================



// ===================================================================
// ========================= PART B ==================================
// Measure USART transmit time @9600 baud using PA4
// ===================================================================
#ifdef RUN_PART_B

    // Part B: USART @ 9600 baud
    _USART_Init_USART2(SystemCoreClock, 9600);          // Part B

    while (1)                                           // Part B
    {
        for (char c = 'A'; c <= 'Z'; c++)               // Part B
        {
            _GPIO_PinSet(GPIOA, 4);                     // Part B: timing start
            _USART_TxByte(USART2, c);                   // Part B: TX occurs here
            _GPIO_PinClear(GPIOA, 4);                   // Part B: timing end

            for (volatile int i = 0; i < 20000; i++);   // Part B: spacing
        }
    }

#endif
// ========================= END PART B ===============================


// ========================= PART C ==================================
// Produce EXACT 16×16 multiplication table (zero padded) @1200 baud
// Matches screenshot from ICA06 instructions.
// ===================================================================
#ifdef RUN_PART_C

    // Set USART to 1200 baud
    _USART_Init_USART2(SystemCoreClock, 1200);

    // Clear terminal
    _USART_ClearScreen(USART2);

    char text[16];

    // ---------------------------------------------------------------
    // Print column headers (001 to 016)
    //  Leading 4 spaces before first header
    // ---------------------------------------------------------------
    _USART_TxString(USART2, "    ");   // 4 spaces for alignment

    for (int col = 1; col <= 16; col++)
    {
        sprintf(text, "%03d ", col);
        _USART_TxString(USART2, text);
    }

    _USART_TxString(USART2, "\r\n");

    // ---------------------------------------------------------------
    // Print each table row
    // ---------------------------------------------------------------
    for (int row = 1; row <= 16; row++)
    {
        // Row label
        sprintf(text, "%03d ", row);
        _USART_TxString(USART2, text);

        // Row products
        for (int col = 1; col <= 16; col++)
        {
            sprintf(text, "%03d ", row * col);
            _USART_TxString(USART2, text);
        }

        _USART_TxString(USART2, "\r\n");
    }

    // Stop here
    while (1);

#endif
// ========================= END PART C ===============================

// ========================= PART D ==================================
// Advanced: 4-digit input, multiplication, decimal + 32-bit binary output
// USART2 @ 38400 baud
// ===================================================================
#ifdef RUN_PART_D

    // Init USART at 38400 baud
    _USART_Init_USART2(SystemCoreClock, 38400);     // Part D

    // Clear terminal
    _USART_ClearScreen(USART2);                     // Part D

    // Title
    _USART_TxStringXY(USART2, 1, 1, "Part D: Multiply Two 4-Digit Numbers\n\n");

    char buf1[6] = {0};   // holds up to 4 digits + NULL
    char buf2[6] = {0};
    int num1 = 0;
    int num2 = 0;
    uint32_t product = 0;
    char out[64];

    // --------------------- Prompt for first number ---------------------
    _USART_TxString(USART2, "Enter first 4-digit number: ");  
    _USART_RxString(USART2, buf1, 5, _USART_RX_ENFORCE_DIGIT);   
    _USART_TxString(USART2, "\r\n");

    // --------------------- Prompt for second number --------------------
    _USART_TxString(USART2, "Enter second 4-digit number: "); 
    _USART_RxString(USART2, buf2, 5, _USART_RX_ENFORCE_DIGIT);   
    _USART_TxString(USART2, "\r\n\r\n");

    // --------------------- Convert string to integer ---------------------
    num1 = 0;
    for (int i = 0; buf1[i] >= '0' && buf1[i] <= '9'; i++)
        num1 = (num1 * 10) + (buf1[i] - '0');

    num2 = 0;
    for (int i = 0; buf2[i] >= '0' && buf2[i] <= '9'; i++)
        num2 = (num2 * 10) + (buf2[i] - '0');

    // --------------------- Multiply ------------------------------------
    product = (uint32_t)num1 * (uint32_t)num2;

    // --------------------- Output decimal result ------------------------
    sprintf(out, "You entered: %d and %d\r\n", num1, num2);
    _USART_TxString(USART2, out);

    sprintf(out, "Product (decimal): %lu\r\n", product);
    _USART_TxString(USART2, out);

    // --------------------- Output 32-bit binary -------------------------
    _USART_TxString(USART2, "Product (binary 32-bit): ");

    for (int b = 31; b >= 0; b--)
    {
        char bit = (product & (1u << b)) ? '1' : '0';
        _USART_TxByte(USART2, bit);

        if ((b % 4) == 0)      // add spacing every nibble
            _USART_TxByte(USART2, ' ');
    }

    _USART_TxString(USART2, "\r\n\r\n");

    while (1);   // stop program
#endif
// ========================= END PART D ===============================
}

/*************************** End of File *****************************/
