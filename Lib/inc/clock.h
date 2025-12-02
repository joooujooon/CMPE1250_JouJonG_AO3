//******************
//Clock Library
//
// CREATED: Sept/24/2024, by Carlos Estay
//
// FILE: clock.h
//
//
#include "stm32g031xx.h"

#ifndef CLOCK_H
#define CLOCK_H

#define RCC_CFGR_SW_PLL RCC_CFGR_SW_1 //PLL as system clock
/*  
  PLL_CLK = PLL_IN x (N / M) / R 
  Minumum R = 2

  R and M factor must subtract 1 as 0 means setting 1
*/
typedef enum PllRangeTypedef__
{             //(R-1)<<RCC_PLLCFGR_PLLR_Pos |  N<<RCC_PLLCFGR_PLLN_Pos | (1-1)<<RCC_PLLCFGR_PLLM_Pos  // PLL_IN   N   M   R
    PLL_16MHZ = (8-1)<<RCC_PLLCFGR_PLLR_Pos |  8<<RCC_PLLCFGR_PLLN_Pos | (1-1)<<RCC_PLLCFGR_PLLM_Pos, // 16MHz x  8 / 1 / 8 = 16MHz
    PLL_20MHZ,
    PLL_32MHZ,
    PLL_40MHZ,
    PLL_48MHZ,
    PLL_50MHZ,
    PLL_60MHz,
    PLL_64MHZ,

}PllRange;  


typedef enum MCO_DivTpedef__
{
    MCO_Div1 = 0U << 28,
    MCO_Div2 = 1U << 28,
    MCO_Div4 = 2U << 28,
    MCO_Div8 = 3U << 28,
    MCO_Div16 = 4U << 28
}MCO_Div;

typedef enum MCO_SelectTpedef__
{
    MCO_Sel_None = 0U << 24,
    MCO_Sel_SYSCLK = 1U << 24,
    MCO_Sel_HSI16 = 2U << 24,
    MCO_Sel_MSI = 3U << 24,
    MCO_Sel_HSE = 4U << 24,
    MCO_Sel_PLL = 5U << 24,
    MCO_Sel_LSI = 6U << 24,
    MCO_Sel_LSE = 7U << 24
}MCO_Select;


void Clock_InitPll(PllRange);
void Clock_EnableOutput(MCO_Select, MCO_Div);
  
#endif /* CLOCK_H */