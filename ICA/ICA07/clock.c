/*********************************************************************
*   CLOCK LIBRARY - ICA07 (Clocks, PLL, MCO)
*   File: clock.h from Carlos Estay 
*   Author : Jou Jon Galenzoga
*********************************************************************/

#include "stm32g031xx.h"
#include "clock.h"

/*********************************************************************
*  INTERNAL: Apply PLL settings from PllRange enum
*********************************************************************/
static void Clock_ApplyPllConfig(PllRange range)
{
    // Disable PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);

    // Apply PLL config:
    // - PLLSRC = HSI16
    // - range (enum) contains (R-1), N, (M-1)
    // - enable PLL R output
    RCC->PLLCFGR =
          RCC_PLLCFGR_PLLSRC_HSI   // HSI16 input
        | (uint32_t)range          // encoded M, N, R from enum
        | RCC_PLLCFGR_PLLREN;      // enable PLL R output

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
}


/*********************************************************************
*  PUBLIC: Initialize PLL and switch SYSCLK to PLL
*********************************************************************/
void Clock_InitPll(PllRange range)
{
    // Flash wait-state required for > 32 MHz
    FLASH->ACR |= FLASH_ACR_LATENCY;

    Clock_ApplyPllConfig(range);

    // Switch SYSCLK to PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Wait until PLL is system clock
  while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (3U << RCC_CFGR_SWS_Pos));
}


/*********************************************************************
*  PUBLIC: Configure MCO Output
*********************************************************************/
void Clock_EnableOutput(MCO_Select sel, MCO_Div div)
{
    uint32_t temp = RCC->CFGR;

    // Clear existing MCO and divider bits
    temp &= ~((7U << 24) | (7U << 28));

    // Apply new settings
    temp |= sel;
    temp |= div;

    RCC->CFGR = temp;
}