/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start
Author: Jou Jon Galenzoga
*/

#include <stdio.h>                 // Used in all parts (A, B, C, D)
#include "stm32g031xx.h"          // Used in all parts (A, B, C, D)
#include "gpio.h"                 // Used in parts B, C, D


void Delay(volatile uint32_t d)   // Used in A, B, C, D
{
    while(d--);                   // Used in A, B, C, D
}

int main(void)
{
    // ============================================================
    // PART A — RAW REGISTER PC6 TOGGLE (~200 ms)
    // ============================================================

   
    //Enable GPIOC clock
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;        // Part A

     //PC6 as output (MODER6 = 01)
    GPIOC->MODER &= ~(3U << (6 * 2));         // Part A
    GPIOC->MODER |=  (1U << (6 * 2));         // Part A

    while(1)                                  // Part A
    {
        GPIOC->ODR ^= (1U << 6);              // Part A
        Delay(600000);                        // Part A
    }
   

    // ============================================================
    // PART B — USING GPIO LIBRARY
    // ============================================================

    //_GPIO_ClockEnable(GPIOC);                 // Part B, C, D
    //_GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);  // Part B, C, D

    // ============================================================
    // PART C — PB0 Output
    // ============================================================
    //_GPIO_ClockEnable(GPIOB);                 // Part C, D
    //_GPIO_SetPinMode(GPIOB, 0, _GPIO_PinMode_Output); // Part C, D

    // ============================================================
    // PART D — PA9 Input State Monitoring
    // ============================================================
    //_GPIO_ClockEnable(GPIOA);                 // Part D
    //_GPIO_SetPinMode(GPIOA, 9, _GPIO_PinMode_Input);  // Part D

    //int lastState = _GPIO_GetPinIState(GPIOA, 9); // Part D
    //int changeCount = 0;                     // Part D

    //while(1)                                 // Part B, C, D
    //{
    //    _GPIO_PinToggle(GPIOC, 6);           // Part B, C, D  (PC6 LED toggle)
    //    Delay(600000);                       // Part A, B, D  (200ms blink)

    //    // _GPIO_PinClear(GPIOB, 0);         // Part C
    //    // Delay(1800000);                   // Part C (600ms LOW)

    //    // _GPIO_PinSet(GPIOB, 0);           // Part C
    //    // Delay(600000);                    // Part C (200ms HIGH)

    //    int now = _GPIO_GetPinIState(GPIOA, 9);   // Part D

    //    if (now != lastState)                // Part D (state change check)
    //    {
    //        changeCount++;                   // Part D
    //        //printf requires SWO or USART   // Part D (not used)
    //        lastState = now;                 // Part D
    //    }
    //}

    return 0;                                // Used in all parts
}

/*************************** End of file ****************************/