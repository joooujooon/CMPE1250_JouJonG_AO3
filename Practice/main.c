/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************

File    : main.c - EXAM MASTER FILE
Author  : Jou Jon Galenzoga

*********************************************************************/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "usart.h"

// ================================================================
// SELECT WHICH EXAM PART TO RUN 
// ================================================================
//#define RUN_PART_A     
//#define RUN_PART_B    
#define RUN_PART_C    

void Delay(volatile uint32_t d)
{
    while(d--);                    // Busy-wait delay loop
}


int main(void)
{
    //-----------------------------------------------------------------
    // COMMON HARDWARE INITIALIZATION
    //-----------------------------------------------------------------
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOB);
    _GPIO_ClockEnable(GPIOC);

    _GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);   // LED on PC6


    // =================================================================
    // ========================= PART A =================================
    // Clear terminal, print name at top-left, course at bottom-right
    // =================================================================
#ifdef RUN_PART_A

    // Init USART @ 38400 baud
    _USART_Init_USART2(SystemCoreClock, 38400);

    // Clear terminal
    _USART_ClearScreen(USART2);

    // Print Name (Top-left Corner)
    _USART_TxStringXY(USART2, 1, 1, "Jou Jon Galenzoga");

    // Print Course (Bottom-right Corner)
    const char *courseText = "CMPE1250";
    int col = _USART_COLS - 7 ;   // "CMPE1250" = 7 chars
    int row = _USART_ROWS;
    _USART_TxStringXY(USART2, col, row, courseText);

    // Idle here
    while (1);

#endif
    // ========================= END PART A ============================




    // =================================================================
    // ========================= PART B =================================
    // Lowercase echo 
    // =================================================================
#ifdef RUN_PART_B

    _USART_Init_USART2(SystemCoreClock, 38400);

    char c;  
    const char CASE_MASK = ('a' ^ 'A');    // bit difference for ASCII case flip

    while (1)
    {
        if (_USART_RxByte(USART2, &c))     // Non-blocking RX
        {
            if (c > ' ')                   // Filter out control/whitespace
            {
                // Convert UPPERCASE → lowercase
                if (c >= 'A' && c <= 'Z')
                    c ^= CASE_MASK;

                _USART_TxByte(USART2, c);
            }
        }

        // ---- Overrun Error Handling ----
        if (USART2->ISR & USART_ISR_ORE)
        {
            if (!(USART2->ISR & USART_ISR_RXNE_RXFNE))
                USART2->ICR |= USART_ICR_ORECF;
        }
    }

#endif
    // ========================= END PART B ============================


// =================================================================
 // ================================================================
// ========================= PART C ==================================
// Part C
// ===================================================================
#ifdef RUN_PART_C


    // Init USART 
    _USART_Init_USART2(SystemCoreClock, 38400);

    uint32_t counter = 0;
    char key = 0;
    char out[16];

    while (1)
    {
        // *** BLOCKING RECEIVE — waits until NEW key is typed ***
        key = _USART_RxByteB(USART2);  

        // ===========================
        // NUMERIC KEY: add to counter
        // ===========================
        if (key >= '0' && key <= '9')
        {
            counter += (key - '0');
        }
        else
        {
          _GPIO_PinToggle(GPIOC, 6);            // Toggle LED
          Delay(600000);                        // ~200ms delay
        }

        // ===========================
        // Print counter as NEW entry
        // ===========================
        sprintf(out, "%06lu\r\n", counter);
        _USART_TxString(USART2, out);
    }

#endif

// ========================= END PART C ===============================
}

/*************************** End of File *****************************/
