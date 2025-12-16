//******************
// Clock Library (ICA07)
// FILE: clock.c
//******************
#include <clock.h>

static void Clock_SetFlashForHclk(uint32_t hclk_hz)
{
    // Enable prefetch for better performance
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    // Set wait states based on rule-of-thumb:
    // 0WS <= 24MHz, 1WS <= 48MHz, 2WS <= 64MHz (Range 1)
    FLASH->ACR &= ~FLASH_ACR_LATENCY;

    if (hclk_hz <= 24000000U)
    {
        // 0 WS -> LATENCY = 0
    }
    else if (hclk_hz <= 48000000U)
    {
        FLASH->ACR |= FLASH_ACR_LATENCY_0;              // 1 WS
    }
    else
    {
        FLASH->ACR |= (FLASH_ACR_LATENCY_0 | FLASH_ACR_LATENCY_1); // 2 WS
    }

    // Ensure write is taken
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != (FLASH->ACR & FLASH_ACR_LATENCY)) {;}
}

void Clock_InitPll(PllRange range)
{
    // Only HSI16 is available as PLL source on STM32G031/G030
    const uint32_t pll_in_hz = 64000000U;

    // Choose M,N,R to hit requested SYSCLK (via PLLR)
    // We keep M=1 always. Then: PLLRCLK = 16MHz * N / R
    // R can be 2,4,6,8 (encoded), but common is 2.
    uint32_t N = 8;
    uint32_t R = 2;
    uint32_t target_hz = (uint32_t)range * 1000000U;

    // Prefer R=2 and compute N directly if possible
    // target = 16 * N / 2  => N = target/8MHz
    if (target_hz % 8000000U == 0U)
    {
        R = 2;
        N = target_hz / 8000000U;
    }
    else if (target_hz % 4000000U == 0U)
    {
        R = 4;
        N = target_hz / 4000000U;
    }
    else
    {
        // Fallback for this ICA: default to 64MHz settings
        target_hz = 64000000U;
        R = 2;
        N = 8;
    }

    // Flash wait states + prefetch before speeding up
    Clock_SetFlashForHclk(target_hz);

    // Disable PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) {;}

    // Build PLLCFGR
    // PLLSRC = HSI16
    // PLLM = /1  => M bits = 0
    // PLLN = N
    // PLLR = R (encoded: 00=/2, 01=/4, 10=/6, 11=/8)
    uint32_t r_enc = 0;
    if      (R == 2) r_enc = 0;
    else if (R == 4) r_enc = 1;
    else if (R == 6) r_enc = 2;
    else             r_enc = 3;

    RCC->PLLCFGR =
          RCC_PLLCFGR_PLLSRC_HSI
        | (0U << RCC_PLLCFGR_PLLM_Pos)
        | ((N & 0x7FU) << RCC_PLLCFGR_PLLN_Pos)
        | (r_enc << RCC_PLLCFGR_PLLR_Pos)
        | RCC_PLLCFGR_PLLREN;   // enable PLLR output

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {;}

    // Switch SYSCLK = PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= (3U << RCC_CFGR_SW_Pos);   // 0b11 = PLL

    // Wait until PLL is system clock (SWS = PLL -> 0b11)
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (3U << RCC_CFGR_SWS_Pos)) {;}

    // Update CMSIS clock variable if available
    SystemCoreClockUpdate();
}

void Clock_EnableOutput(MCO_Select src, MCO_Div div)
{
    // If LSI is requested, turn it on first (required)
    if (src == MCO_Sel_LSI)
    {
        RCC->CSR |= RCC_CSR_LSION;
        while (!(RCC->CSR & RCC_CSR_LSIRDY)) {;}
    }

    // Configure MCOSEL + MCOPRE
    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_Msk | RCC_CFGR_MCOPRE_Msk);
    RCC->CFGR |= (uint32_t)src | (uint32_t)div;
}
