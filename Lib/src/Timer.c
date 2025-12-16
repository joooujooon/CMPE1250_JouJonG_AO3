#include "timer.h"                                                                // include own header

//==================================================================================================
// TIM14: INIT 1MHz TICK (1us per count) like your demo: PSC = (SYSCLK/1MHz)-1
//==================================================================================================
void Timer14_Init_1MHzTick(uint32_t sysclkHz)                                     // init TIM14 for 1us ticks
{                                                                                 // start function
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;                                          // enable TIM14 clock on APB2

    TIM14->CR1 &= ~TIM_CR1_CEN;                                                   // disable timer during config
    TIM14->PSC = (uint16_t)((sysclkHz / 1000000u) - 1u);                          // prescaler to make 1MHz timer clock
    TIM14->ARR = 0xFFFFu;                                                        // max auto-reload (free-running)
    TIM14->CNT = 0u;                                                             // reset counter
    TIM14->EGR = TIM_EGR_UG;                                                      // force update (load PSC/ARR)
    TIM14->SR  = 0u;                                                              // clear all flags
    TIM14->CR1 |= TIM_CR1_CEN;                                                    // enable timer
}                                                                                 // end function

//==================================================================================================
// TIM14: BLOCKING DELAY IN MICROSECONDS (polling, demo-style)
//==================================================================================================
void Timer14_Delay_us(uint16_t us)                                                // delay N microseconds
{                                                                                 // start function
    uint16_t start = (uint16_t)TIM14->CNT;                                        // capture start count
    while ((uint16_t)(TIM14->CNT - start) < us) { }                               // wait until elapsed >= us
}                                                                                 // end function

//==================================================================================================
// TIM14: BLOCKING DELAY IN MILLISECONDS (built on microseconds)
//==================================================================================================
void Timer14_Delay_ms(uint16_t ms)                                                // delay N milliseconds
{                                                                                 // start function
    while (ms--)                                                                  // loop ms times
    {                                                                             // start loop
        Timer14_Delay_us(1000u);                                                  // 1000us = 1ms
    }                                                                             // end loop
}                                                                                 // end function
