/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************

File    : main.c - ICA04 (A–D Combined)
Author  : Jou Jon Galenzoga
Purpose : Unified file for ICA04 using #define toggles
*********************************************************************/

#include <stdio.h>               
#include "stm32g031xx.h"         
#include "gpio.h"                


// ===================================================================
// Simple blocking delay (used in multiple parts)
// ===================================================================
void Delay(volatile uint32_t d)
{
    while(d--);                    // Busy-wait delay loop
}


// ===================================================================
// PART SELECTOR — Uncomment the part you want to run
// ===================================================================
//#define RUN_PART_A
//#define RUN_PART_B
//#define RUN_PART_C
#define RUN_PART_D                 // <--- Active part



int main(void)
{

// ===================================================================
// =========================== PART A ================================
// Raw register LED blink using PC6 (~200ms)
// ===================================================================
#ifdef RUN_PART_A

    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;        // Enable GPIOC clock (raw register)

    GPIOC->MODER &= ~(3U << (6 * 2));         // Clear mode bits for PC6
    GPIOC->MODER |=  (1U << (6 * 2));         // PC6 = Output mode

    while(1)
    {
        GPIOC->ODR ^= (1U << 6);              // Toggle PC6
        Delay(600000);                        // ~200ms delay
    }

#endif
// =========================== END PART A ============================



// ===================================================================
// =========================== PART B ================================
// PC6 LED blink using GPIO library
// ===================================================================
#ifdef RUN_PART_B

    _GPIO_ClockEnable(GPIOC);                 // Enable GPIOC clock
    _GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);   // Set PC6 as output

    while(1)
    {
        _GPIO_PinToggle(GPIOC, 6);            // Toggle LED
        Delay(600000);                        // ~200ms delay
    }

#endif
// =========================== END PART B ============================



// ===================================================================
// =========================== PART C ================================
// PB0 Output pulse (200ms HIGH, 600ms LOW)
// ===================================================================
#ifdef RUN_PART_C

    _GPIO_ClockEnable(GPIOB);                 // Enable GPIOB clock
    _GPIO_SetPinMode(GPIOB, 0, _GPIO_PinMode_Output);  // PB0 = output

    while(1)
    {
        _GPIO_PinSet(GPIOB, 0);               // Set PB0 HIGH
        Delay(600000);                        // ON 200ms

        _GPIO_PinClear(GPIOB, 0);             // Set PB0 LOW
        Delay(1800000);                       // OFF 600ms
    }

#endif
// =========================== END PART C ============================



// ===================================================================
// =========================== PART D ================================
// Monitor PA9 input for state changes
// ===================================================================
#ifdef RUN_PART_D

    _GPIO_ClockEnable(GPIOA);                 // Enable GPIOA clock
    _GPIO_SetPinMode(GPIOA, 9, _GPIO_PinMode_Input);    // PA9 as input

    _GPIO_ClockEnable(GPIOC);                 // LED visualization
    _GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);   // PC6 as output

    int lastState = _GPIO_GetPinIState(GPIOA, 9);   // Read initial state
    int changeCount = 0;                           // Counter variable

    while(1)
    {
        _GPIO_PinToggle(GPIOC, 6);            // Visual heartbeat LED
        Delay(600000);                        // ~200ms delay

        int now = _GPIO_GetPinIState(GPIOA, 9);    // Read current state

        if (now != lastState)                 // Detect change
        {
            changeCount++;                    // Increment change count
            lastState = now;                  // Update saved state
            // NOTE: Printing requires SWO/USART
        }
    }

#endif
// =========================== END PART D ============================


    return 0;                                  // End of main
}

/*************************** End of File *****************************/
