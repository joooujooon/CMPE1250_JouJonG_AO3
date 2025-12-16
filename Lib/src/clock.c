#include <clock.h>                                                               // include own header

//==================================================================================================
// INTERNAL HELPERS
//==================================================================================================
static void Clock_EnableHSI16(void)                                               // ensure HSI16 is ON
{                                                                                 // start function
    RCC->CR |= RCC_CR_HSION;                                                      // turn on HSI
    while ((RCC->CR & RCC_CR_HSIRDY) == 0u) { }                                   // wait until ready
}                                                                                 // end function

static void Clock_EnableLSI(void)                                                 // ensure LSI is ON (needed for Part C)
{                                                                                 // start function
    RCC->CSR |= RCC_CSR_LSION;                                                    // enable LSI oscillator
    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0u) { }                                 // wait until LSI ready
}                                                                                 // end function

static void Clock_SetFlashForHighSpeed(uint32_t sysclkHz)                         // set flash wait states for faster clocks
{                                                                                 // start function
    (void)sysclkHz;                                                               // unused in this simple version
    FLASH->ACR |= FLASH_ACR_PRFTEN;                                               // enable prefetch (matches demo style)
    FLASH->ACR &= ~FLASH_ACR_LATENCY;                                             // clear latency bits
    FLASH->ACR |= FLASH_ACR_LATENCY_1;                                            // 1 wait-state (safe up to 64 MHz on G0)
}                                                                                 // end function

static void Clock_SwitchSysclkToHSI(void)                                         // temporarily switch SYSCLK to HSI
{                                                                                 // start function
    RCC->CFGR &= ~RCC_CFGR_SW;                                                    // clear SW bits
    RCC->CFGR |= RCC_CFGR_SW_0;                                                   // SW = 01 (HSI selected as SYSCLK)
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0) { }                      // wait until switch is active
}                                                                                 // end function

static void Clock_DisablePLL(void)                                                // safely disable PLL before reconfig
{                                                                                 // start function
    RCC->CR &= ~RCC_CR_PLLON;                                                     // turn PLL off
    while ((RCC->CR & RCC_CR_PLLRDY) != 0u) { }                                   // wait until PLL not ready
}                                                                                 // end function

static uint32_t Clock_ComputePLLN(uint32_t targetMHz, uint32_t pllR)              // compute N for 16MHz source
{                                                                                 // start function
    return (targetMHz * pllR) / 16u;                                              // target = 16 * N / R -> N = target*R/16
}                                                                                 // end function

static uint32_t Clock_PLLR_ToBits(uint32_t pllR)                                  // convert R divider to PLLR bits
{                                                                                 // start function
    if (pllR == 2u) return 0u;                                                    // 00 = /2
    if (pllR == 4u) return 1u;                                                    // 01 = /4
    if (pllR == 6u) return 2u;                                                    // 10 = /6
    return 3u;                                                                    // 11 = /8 (default fallback)
}                                                                                 // end function

static uint32_t Clock_MCOPreBits(MCO_Div div)                                     // convert /1,/2,/4.. to MCOPRE bits
{                                                                                 // start function
    switch (div)                                                                  // choose prescaler
    {                                                                             // open switch
        case MCO_Div1:   return 0u;                                               // 000 = /1
        case MCO_Div2:   return 1u;                                               // 001 = /2
        case MCO_Div4:   return 2u;                                               // 010 = /4
        case MCO_Div8:   return 3u;                                               // 011 = /8
        case MCO_Div16:  return 4u;                                               // 100 = /16
        case MCO_Div32:  return 5u;                                               // 101 = /32
        case MCO_Div64:  return 6u;                                               // 110 = /64
        case MCO_Div128: return 7u;                                               // 111 = /128
        default:         return 0u;                                               // fallback /1
    }                                                                             // end switch
}                                                                                 // end function

static uint32_t Clock_MCOSelBits(MCO_Select src)                                  // convert select to MCOSEL bits
{                                                                                 // start function
    switch (src)                                                                  // choose source
    {                                                                             // open switch
        case MCO_Sel_SYSCLK: return 1u;                                           // 001 = SYSCLK (common on STM32G0)
        case MCO_Sel_HSI16:  return 3u;                                           // 011 = HSI16 (common on STM32G0)
        case MCO_Sel_LSI:    return 6u;                                           // 110 = LSI (common on STM32G0)
        case MCO_Sel_PLLR:   return 5u;                                           // 101 = PLLRCLK (common on STM32G0)
        default:             return 0u;                                           // 000 = disabled
    }                                                                             // end switch
}                                                                                 // end function

//==================================================================================================
// PUBLIC API
//==================================================================================================
void Clock_InitPll(PLL_ClockFreq target)                                          // configure SYSCLK = target MHz
{                                                                                 // start function
    uint32_t targetMHz = (uint32_t)target;                                        // convert enum to number
    uint32_t pllR = 2u;                                                           // choose R=2 for nice results (demo-friendly)

    if (targetMHz == 16u)                                                         // if they want 16MHz
    {                                                                             // start if
        Clock_EnableHSI16();                                                      // ensure HSI ready
        Clock_SwitchSysclkToHSI();                                                // SYSCLK = HSI
        SystemCoreClock = 16000000u;                                              // update global
        return;                                                                   // done
    }                                                                             // end if

    Clock_EnableHSI16();                                                          // make sure HSI16 is running
    Clock_SetFlashForHighSpeed(targetMHz * 1000000u);                             // set flash latency/prefetch

    Clock_SwitchSysclkToHSI();                                                    // must switch away from PLL before editing it
    Clock_DisablePLL();                                                          // disable PLL so we can reconfigure

    uint32_t N = Clock_ComputePLLN(targetMHz, pllR);                              // compute PLLN

    if (N < 8u)  N = 8u;                                                          // clamp N low (datasheet safe range)
    if (N > 86u) N = 86u;                                                         // clamp N high (datasheet safe range)

    RCC->PLLCFGR = 0u;                                                            // clear PLLCFGR (simple reset style)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;                                       // PLL source = HSI16
    RCC->PLLCFGR |= (0u << RCC_PLLCFGR_PLLM_Pos);                                 // PLLM = /1 (M=0 means /1 on G0)
    RCC->PLLCFGR |= (N  << RCC_PLLCFGR_PLLN_Pos);                                 // set PLLN
    RCC->PLLCFGR |= (Clock_PLLR_ToBits(pllR) << RCC_PLLCFGR_PLLR_Pos);            // set PLLR divider bits
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;                                           // enable PLLR output

    RCC->CR |= RCC_CR_PLLON;                                                      // enable PLL
    while ((RCC->CR & RCC_CR_PLLRDY) == 0u) { }                                   // wait until PLL locked

    RCC->CFGR &= ~RCC_CFGR_SW;                                                    // clear SYSCLK switch bits
    RCC->CFGR |= RCC_CFGR_SW_1;                                                   // SW = 10 (PLL selected as SYSCLK)
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_1) { }                      // wait until PLL is SYSCLK

    SystemCoreClock = targetMHz * 1000000u;                                       // update SystemCoreClock variable
}                                                                                 // end function

void Clock_EnableOutput(MCO_Select src, MCO_Div div)                              // route clock source to MCO pin
{                                                                                 // start function
    if (src == MCO_Sel_LSI)                                                       // if using LSI
    {                                                                             // start if
        Clock_EnableLSI();                                                        // ensure LSI oscillator is ON
    }                                                                             // end if

    uint32_t cfgr = RCC->CFGR;                                                    // read CFGR
    cfgr &= ~RCC_CFGR_MCOSEL;                                                     // clear MCO source bits
    cfgr &= ~RCC_CFGR_MCOPRE;                                                     // clear MCO prescaler bits

    cfgr |= (Clock_MCOSelBits(src) << RCC_CFGR_MCOSEL_Pos);                       // set MCO source bits
    cfgr |= (Clock_MCOPreBits(div) << RCC_CFGR_MCOPRE_Pos);                       // set MCO prescaler bits

    RCC->CFGR = cfgr;                                                             // write back CFGR
}                                                                                 // end function

uint32_t Clock_GetSysclkHz(void)                                                  // return current SYSCLK
{                                                                                 // start function
    return SystemCoreClock;                                                       // return global core clock
}                                                                                 // end function
