/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************
File    : main.c - ICA05 
Author  : Jou Jon Galenzoga

*/

#include <stdio.h>                     
#include "stm32g031xx.h"                
#include "gpio.h"                       
#include "usart.h"                      

// ================================================================
// SELECT WHICH PART TO RUN (UNCOMMENT ONLY ONE)
// Each part demonstrates a specific USART concept.
// ================================================================
#define RUN_PART_A                   // Heartbeat transmitter
//#define RUN_PART_B                   // Basic echo test
//#define RUN_PART_C                   // Smart delay + polling
//#define RUN_PART_D                   // Circular buffer + LED logic
//#define RUN_PART_E                      // Uppercase + overrun handling


int main(void)                          // Program entry point
{
    //-----------------------------------------------------------------
    // COMMON INITIALIZATION (used by ALL parts)
    //-----------------------------------------------------------------
    _GPIO_ClockEnable(GPIOA);           // Enable GPIOA clock (needed for PA4, PA2/PA3)
    _GPIO_ClockEnable(GPIOB);           // Enable GPIOB clock (needed for PB0)
    _GPIO_ClockEnable(GPIOC);           // Enable GPIOC clock (needed for PC6 LED)

    _GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);  // PC6 LED, used in Part D/E
    _GPIO_SetPinMode(GPIOB, 0, _GPIO_PinMode_Output);  // PB0 timing pin for Part C
    _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_Output);  // PA4 optional debug pin

    //-----------------------------------------------------------------
    // ========================= PART A ================================
    // HEARTBEAT — TX a '.' every ~200ms
    //-----------------------------------------------------------------
#ifdef RUN_PART_A                    // Only compile this section if Part A is selected

    int countA = 0;                  // Counter used to time heartbeat
    int delayA = 10000;              // Delay threshold for ~200ms

    _USART_Init_USART2(SystemCoreClock, 38400);   // Initialize USART2 @ 38400 baud

    while (1)                        // Forever loop
    {
        if (!(countA % delayA))     // If counter hits the delay interval
        {
            _USART_TxByte(USART2, '.');   // Send heartbeat dot
        }
        countA++;                    // Increment counter continuously
    }

#endif
// ========================= END PART A ===============================



// ===================================================================
// ========================= PART B ==================================
// BASIC RX ECHO — Reads characters & immediately echoes them back
// ===================================================================
#ifdef RUN_PART_B

    _USART_Init_USART2(SystemCoreClock, 38400);   // Initialize USART2 @ 38400 baud

    char rx;                                      // Variable to store received byte

    while (1)                                     // Infinite loop
    {
        if (_USART_RxByte(USART2, &rx))           // If character received (non-blocking)
        {
            printf("%c", rx);                     // Print to debug console (optional)
            _USART_TxByte(USART2, rx);            // Send character back to terminal
        }
    }

#endif
// ========================= END PART B ===============================



// ===================================================================
// ========================= PART C ==================================
// SMART DELAY LOOP — Poll for RX during delay, toggle PB0 for scope
// ===================================================================
#ifdef RUN_PART_C

    int count = 0;                    // Loop counter (not used heavily)
    int delay = 5000;                 // Smart delay interval
    char rxC = 0;                     // Temp variable for received byte

    _USART_Init_USART2(SystemCoreClock, 38400);   // Init USART @ 38400 baud

    while (1)                        // Infinite loop
    {
        for (int d = 0; d < delay; d++)   // "Delay" loop — but we poll inside
        {
            if (_USART_RxByte(USART2, &rxC))   // Check for RX inside delay loop
            {
                _USART_TxByte(USART2, rxC);    // Echo if received
            }
        }

        _GPIO_PinToggle(GPIOB, 0);    // Toggle PB0 so oscilloscope shows loop timing
        count++;                      // Increment loop counter
    }

#endif
// ========================= END PART C ===============================



// ===================================================================
// ========================= PART D ==================================
// CIRCULAR BUFFER — Store last 5 chars, compare ASCII sum to threshold
// LED ON when sum >= midpoint * 5
// ===================================================================
#ifdef RUN_PART_D

    _USART_Init_USART2(SystemCoreClock, 38400);   // Init USART @ 38400 baud

    char cD = 0;                                  // Variable for received character
    int bufindx = 0;                              // Index of ring buffer
    int charcount = 0;                            // Total characters received
    int avealpha = ('a' + 'z') / 2;               // ASCII midpoint of a–z
    int sumbuf = 0;                               // Sum of ASCII values
    char buffer[5] = {0};                         // 5-character circular buffer

    while (1)
    {
        if (_USART_RxByte(USART2, &cD))           // If a character arrives
        {
            buffer[bufindx] = cD;                 // Store into ring buffer
            bufindx = (bufindx + 1) % 5;          // Wrap index 0–4
            charcount++;                          // Increase character count

            if (charcount >= 5)                   // Only compute once buffer is full
            {
                sumbuf = 0;                       // Reset sum

                for (int i = 0; i < 5; i++)       // Add ASCII of last 5 chars
                    sumbuf += buffer[i];

                if (sumbuf >= avealpha * 5)       // Compare to threshold
                    _GPIO_PinSet(GPIOC, 6);       // LED ON
                else
                    _GPIO_PinClear(GPIOC, 6);     // LED OFF
            }

            _USART_TxByte(USART2, cD);            // Echo received character
        }
    }

#endif
// ========================= END PART D ===============================



// ===================================================================
// ========================= PART E ==================================
// UPPERCASE + OVERRUN HANDLING
// Convert a–z → A–Z, ignore whitespace, detect and clear ORE
// ===================================================================
#ifdef RUN_PART_E

    _USART_Init_USART2(SystemCoreClock, 38400);   // Init USART @ 38400 baud

    char cE = 0;                                   // Received character
    const char CASE_MASK = ('a' ^ 'A');            // XOR mask to flip case bit

    while (1)
    {
        //---------------------------- RECEIVE ----------------------------
        if (_USART_RxByte(USART2, &cE))            // Non-blocking RX
        {
            if (cE > ' ')                          // Ignore whitespace/control
            {
                if (cE >= 'a' && cE <= 'z')        // If lowercase letter
                    cE ^= CASE_MASK;               // Convert to uppercase

                _USART_TxByte(USART2, cE);         // Echo processed character
            }
        }

        //---------------------------- ORE CHECK ---------------------------
        if (USART2->ISR & USART_ISR_ORE)           // If overrun error flagged
        {
            printf("\nOverrun!\n");                // Debug message

            if (!(USART2->ISR & USART_ISR_RXNE_RXFNE)) // If no unread data
                USART2->ICR |= USART_ICR_ORECF;    // Clear ORE flag
        }
    }

#endif
// ========================= END PART E ===============================

}

/*************************** End of File *****************************/
