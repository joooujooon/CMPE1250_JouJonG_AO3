//******************
// Clock Library (ICA07)
// FILE: clock.h
//******************
#include "stm32g031xx.h"

#ifndef CLOCK_H
#define CLOCK_H

/*  PLL_CLK = PLL_IN x (N / M) / R
    On STM32G031: PLL_IN is typically HSI16 (no HSE on G031/G030).
*/

typedef enum PllRangeTypedef__
{
    PLL_16MHZ = 16,
    PLL_20MHZ = 20,
    PLL_32MHZ = 32,
    PLL_40MHZ = 40,
    PLL_48MHZ = 48,
    PLL_50MHZ = 50,
    PLL_60MHZ = 60,
    PLL_64MHZ = 64
} PllRange;

typedef enum MCO_DivTpedef__
{
    MCO_Div1   = (0U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div2   = (1U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div4   = (2U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div8   = (3U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div16  = (4U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div32  = (5U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div64  = (6U << RCC_CFGR_MCOPRE_Pos),
    MCO_Div128 = (7U << RCC_CFGR_MCOPRE_Pos)
} MCO_Div;

typedef enum MCO_SelectTpedef__
{
    MCO_Sel_None   = (0U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_SYSCLK = (1U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_HSI16  = (2U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_MSI    = (3U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_HSE    = (4U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_PLL    = (5U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_LSI    = (6U << RCC_CFGR_MCOSEL_Pos),
    MCO_Sel_LSE    = (7U << RCC_CFGR_MCOSEL_Pos)
} MCO_Select;

void Clock_InitPll(PllRange range);
void Clock_EnableOutput(MCO_Select src, MCO_Div div);

#endif
