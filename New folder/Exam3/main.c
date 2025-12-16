/*********************************************************************
*  ICA07 - Clocks, PLL, and MCO
*  CMPE1250 - Jou Jon Galenzoga
*
*  PART A  – Baseline (HSI16) + MCO
*  PART B  – SYSCLK = 64 MHz using PLL (same delayCount)
*  PART C  – MCO = LSI / 64 on PA8
*********************************************************************/

#include "stm32g031xx.h"
#include "gpio.h"
#include "usart.h"
#include <stdio.h>

#define SEGGER_RTT_DISABLE

// =======================================================
// SELECT ONE PART ONLY
// =======================================================
#define RUN_PART_A
//#define RUN_PART_B
// #define RUN_PART_C

// =======================================================
// RCC_CFGR MCO FIELD MASKS (RM0444 5.4.3)
// =======================================================
#define RCC_CFGR_MCOSEL_MASK   (0xFU << 24)
#define RCC_CFGR_MCOPRE_MASK   (0xFU << 28)

// MCOSEL codes (RM0444 5.4.3)
#define MCOSEL_NONE            (0x0U << 24)
#define MCOSEL_SYSCLK          (0x1U << 24)
#define MCOSEL_HSI16           (0x3U << 24)
#define MCOSEL_PLLRCLK         (0x5U << 24)
#define MCOSEL_LSI             (0x6U << 24)
#define MCOSEL_LSE             (0x7U << 24)

// MCOPRE codes (RM0444 5.4.3) 0000=1,0001=2,0010=4,...,0111=128
#define MCOPRE_DIV1            (0x0U << 28)
#define MCOPRE_DIV2            (0x1U << 28)
#define MCOPRE_DIV4            (0x2U << 28)
#define MCOPRE_DIV8            (0x3U << 28)
#define MCOPRE_DIV16           (0x4U << 28)
#define MCOPRE_DIV32           (0x5U << 28)
#define MCOPRE_DIV64           (0x6U << 28)
#define MCOPRE_DIV128          (0x7U << 28)

// =======================================================
// GLOBALS
// =======================================================
static uint32_t counter = 0;

// =======================================================
// SIMPLE DELAY
// =======================================================
static void Delay(volatile uint32_t d)
{
    while (d--) __NOP();
}

// =======================================================
// GPIO INIT: PA5, PC6 outputs; PA8 AF0 for MCO
// =======================================================
static void ICA07_InitGPIO(void)
{
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOC);

    GPIO_InitOutput(GPIOA, 0); //  For Oscilloscope
    GPIO_InitOutput(GPIOC, 6); // PC6

    // PA8 = AF0 (MCO)
    _GPIO_SetPinMode(GPIOA, 8, _GPIO_PinMode_AlternateFunction);
    GPIOA->AFR[1] &= ~(0xFU << 0); // AF0 for PA8
}

// =======================================================
// PLL INIT 64 MHz (HSI16 * 8 / 2)
// =======================================================
static void PLL_Init_64MHz(void)
{
    // 1) Flash: prefetch + wait states (SAFE for 64MHz)
    //    NOTE: 1 wait state may cause lock-up at 64MHz on some boards,
    //    so we set 2 wait states to be safe.
    FLASH->ACR |= FLASH_ACR_PRFTEN;
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= (FLASH_ACR_LATENCY_0 | FLASH_ACR_LATENCY_1); // 2 WS SAFE

    // 2) Ensure HSI16 is ON (PLL source)
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // 3) Disable PLL before configuration
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);

    // 4) Configure PLL = 64MHz from HSI16:
    //    M=1, N=8, R=2  => 16*8/2 = 64MHz
    RCC->PLLCFGR = 0; // clear everything first (prevents leftover config)
    RCC->PLLCFGR =
          RCC_PLLCFGR_PLLSRC_HSI
        | (0U << RCC_PLLCFGR_PLLM_Pos)   // M=/1
        | (8U << RCC_PLLCFGR_PLLN_Pos)   // N=8
        | (0U << RCC_PLLCFGR_PLLR_Pos)   // R=/2
        | RCC_PLLCFGR_PLLREN;            // enable R output

    // 5) Enable PLL and wait ready
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // 6) Switch SYSCLK to PLL (SW=0b11) and wait (SWS=0b11)
    //    NOTE: STM32G0 uses SW=0b11 for PLL (no RCC_CFGR_SW_PLL macro)
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= (3U << RCC_CFGR_SW_Pos);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (3U << RCC_CFGR_SWS_Pos));

    // 7) Update SystemCoreClock (CMSIS variable)
    SystemCoreClockUpdate();
}

// =======================================================
// PART A — BASELINE (HSI16)
// =======================================================
static void ICA07_PartA(void)
{
    _USART_Init_USART2(16000000U, 115200U);
    SystemCoreClockUpdate();

    // MCO = HSI16 / 16  (MCOSEL=0011, MCOPRE=0100)
    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_MASK | RCC_CFGR_MCOPRE_MASK);
    RCC->CFGR |=  (MCOSEL_HSI16 | MCOPRE_DIV16);

    const uint32_t delayCount = 1600000U; // tune for ~200ms @ 16MHz // Divide the Freq to find the new delay time. 

    // PA8 = MCO (Alternate Function 0)
    _GPIO_SetPinMode(GPIOA, 8, _GPIO_PinMode_AlternateFunction);
    GPIOA->AFR[1] &= ~(0xFU << 0);   // AFRH[3:0] for PA8 = AF0

    // -------------------------------------------------------
    // MCO = SYSCLK / 128 (RM0444 5.4.3)  [Handout Snippet]
    // MCOSEL=0001 (SYSCLK), MCOPRE=0111 (/128)
    // -------------------------------------------------------
    unsigned int cfgrtemp = RCC->CFGR;
    cfgrtemp &= 0b00000000000000000000000000111111;
    cfgrtemp |= 0b01110001000000000000000000000000;
    RCC->CFGR = cfgrtemp;

    while (1)
    {
        GPIO_Toggle(GPIOA, 0);
        GPIO_Toggle(GPIOC, 6);

        printf("Tick A  Count=%lu  SYSCLK=%lu Hz\n",
               counter++, (uint32_t)SystemCoreClock);

        Delay(delayCount);
    }
}

//// =======================================================
//// PART B — PLL 64 MHz (SAME delayCount as Part A)
//// =======================================================
//static void ICA07_PartB(void)
//{
//    // -------------------------------------------------------
//    // Quick "alive" blink BEFORE PLL (debug helper)
//    // If you see this then it stops, PLL config is where it hangs.
//    // -------------------------------------------------------
//    //GPIO_Toggle(GPIOC, 6);
//    //Delay(200000);

//    // -------------------------------------------------------
//    // As the VERY FIRST ONE-TIME INIT in Part B:
//    // Increase SYSCLK to 64MHz using PLL
//    // -------------------------------------------------------
//    //PLL_Init_64MHz();

//    // Update CMSIS clock variable again (safe) before using it/printing
//    SystemCoreClockUpdate();

//    // USART must be re-initialized using the NEW SYSCLK to keep 115200 correct
//    _USART_Init_USART2(64000000U, 115200U);

//    // MCO = SYSCLK / 1 (fast)  (MCOSEL=0001, MCOPRE=0000)
//    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_MASK | RCC_CFGR_MCOPRE_MASK);
//    RCC->CFGR |=  (MCOSEL_SYSCLK | MCOPRE_DIV1);

//    // IMPORTANT: SAME delayCount as Part A (so blink becomes faster at 64MHz)
//    const uint32_t delayCount = 6400000U;

//    while (1)
//    {
//        GPIO_Toggle(GPIOA, 0); // PA0 scope
//        GPIO_Toggle(GPIOC, 6); // PC6 

//        printf("Tick B  Count=%lu  SYSCLK=%lu Hz\n",
//               counter++, (uint32_t)SystemCoreClock);

//        Delay(delayCount);
//    }
//}

//// =======================================================
//// PART C — MCO = LSI / 64 (PA8)
//// =======================================================
//static void ICA07_PartC(void)
//{
//    PLL_Init_64MHz();
//    SystemCoreClockUpdate();
//    _USART_Init_USART2(64000000U, 115200U);

//    // Enable LSI
//    RCC->CSR |= RCC_CSR_LSION;
//    while (!(RCC->CSR & RCC_CSR_LSIRDY));

//    // MCO = LSI / 64 (MCOSEL=0110, MCOPRE=0110)
//    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_MASK | RCC_CFGR_MCOPRE_MASK);
//    RCC->CFGR |=  (MCOSEL_LSI | MCOPRE_DIV64);

//    const uint32_t delayCount = 200000U;
//    // ---------------------------
//    // Enable LSI 
//    // ---------------------------
//    RCC->CSR |= RCC_CSR_LSION;
//    while (!(RCC->CSR & RCC_CSR_LSIRDY));   // wait LSI ready

//    // ---------------------------
//    // MCO = LSI / 64 on PA8
//    // MCOSEL = LSI, MCOPRE = /64  (RM0444 5.4.3)
//    // ---------------------------
//    RCC->CFGR &= ~(RCC_CFGR_MCOSEL_MASK | RCC_CFGR_MCOPRE_MASK);
//    RCC->CFGR |=  (MCOSEL_LSI | MCOPRE_DIV64);

//    while (1)
//    {
//        GPIO_Toggle(GPIOA, 5);
//        GPIO_Toggle(GPIOC, 6);

//        printf("Tick C  Count=%lu  SYSCLK=%lu Hz  (MCO=LSI/64)\n",
//               counter++, (uint32_t)SystemCoreClock);

//        Delay(delayCount);
//    }
//}

// =======================================================
// MAIN
// =======================================================
int main(void)
{
    ICA07_InitGPIO();

#ifdef RUN_PART_A
    ICA07_PartA();
#endif

#ifdef RUN_PART_B
    ICA07_PartB();
#endif

#ifdef RUN_PART_C
    ICA07_PartC();
#endif

    while (1) __WFI();
}

