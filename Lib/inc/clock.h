#ifndef CLOCK_H                                                                 // include guard start
#define CLOCK_H                                                                 // include guard define

#include "stm32g031xx.h"                                                        // STM32G031 register definitions
#include <stdint.h>                                                              // uint32_t types

//==================================================================================================
// PLL TARGET OPTIONS (SYSCLK after Clock_InitPll)
//==================================================================================================
typedef enum                                                                     // enum start
{                                                                                // open enum
    PLL_16MHZ = 16,                                                              // SYSCLK = 16 MHz (no PLL switch needed)
    PLL_24MHZ = 24,                                                              // SYSCLK = 24 MHz (PLL)
    PLL_32MHZ = 32,                                                              // SYSCLK = 32 MHz (PLL)
    PLL_40MHZ = 40,                                                              // SYSCLK = 40 MHz (PLL demo-style)
    PLL_48MHZ = 48,                                                              // SYSCLK = 48 MHz (PLL)
    PLL_56MHZ = 56,                                                              // SYSCLK = 56 MHz (PLL)
    PLL_64MHZ = 64                                                               // SYSCLK = 64 MHz (ICA07 requirement)
} PLL_ClockFreq;                                                                 // enum name

//==================================================================================================
// MCO OUTPUT SELECT + DIV OPTIONS (RCC->CFGR MCO on PA8)
//==================================================================================================
typedef enum                                                                     // enum start
{                                                                                // open enum
    MCO_Sel_Disabled = 0,                                                        // disable MCO output
    MCO_Sel_SYSCLK,                                                              // output SYSCLK
    MCO_Sel_HSI16,                                                               // output HSI16
    MCO_Sel_LSI,                                                                 // output LSI
    MCO_Sel_PLLR                                                                 // output PLLRCLK (PLL output)
} MCO_Select;                                                                    // enum name

typedef enum                                                                     // enum start
{                                                                                // open enum
    MCO_Div1   = 1,                                                              // divide by 1
    MCO_Div2   = 2,                                                              // divide by 2
    MCO_Div4   = 4,                                                              // divide by 4
    MCO_Div8   = 8,                                                              // divide by 8
    MCO_Div16  = 16,                                                             // divide by 16
    MCO_Div32  = 32,                                                             // divide by 32
    MCO_Div64  = 64,                                                             // divide by 64
    MCO_Div128 = 128                                                             // divide by 128
} MCO_Div;                                                                       // enum name

//==================================================================================================
// API
//==================================================================================================
void Clock_InitPll(PLL_ClockFreq target);                                         // set SYSCLK using HSI->PLL
void Clock_EnableOutput(MCO_Select src, MCO_Div div);                             // route clock to MCO (PA8)
uint32_t Clock_GetSysclkHz(void);                                                 // return SystemCoreClock

#endif                                                                            // include guard end
