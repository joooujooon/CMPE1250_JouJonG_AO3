#ifndef TIMER_H                                                                 // include guard start
#define TIMER_H                                                                 // include guard define

#include "stm32g031xx.h"                                                        // STM32 register definitions
#include <stdint.h>                                                              // uint32_t types

//==================================================================================================
// TIM14 SIMPLE POLLING TIMER (matches your demo style: PSC, ARR, poll UIF/CC1IF)
//==================================================================================================
void Timer14_Init_1MHzTick(uint32_t sysclkHz);                                    // set TIM14 tick = 1us
void Timer14_Delay_us(uint16_t us);                                               // blocking microsecond delay (poll UIF)
void Timer14_Delay_ms(uint16_t ms);                                               // blocking millisecond delay (built on us)

#endif                                                                           // include guard end
