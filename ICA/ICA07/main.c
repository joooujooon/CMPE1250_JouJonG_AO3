/*********************************************************************
*  ICA07 - Clocks, PLL, and MCO
*  CMPE1250 - Jou Jon Galenzoga
*********************************************************************/

#include "stm32g031xx.h"
#include "gpio.h"
#include "clock.h"
#include "usart.h"
#include <stdio.h>

#define SEGGER_RTT_DISABLE

// =======================================================
// SELECT ONE PART ONLY
// =======================================================
//#define RUN_PART_A
//#define RUN_PART_B
#define RUN_PART_C

static uint32_t counter = 0;
static uint32_t time_ms = 0;


// =======================================================
// SIMPLE DELAY
// =======================================================
static void Delay(volatile uint32_t d)
{
    while (d--) __NOP();
}


// =======================================================
// MANUAL PLL INIT (KNOWN-WORKING 64 MHz CONFIG)
// =======================================================
static void PLL_Init_64MHz(void)
{
    FLASH->ACR |= FLASH_ACR_LATENCY;    // 1 WS required for >32MHz

    // Turn off PLL
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);

    // Configure PLL:
    // Source = HSI16
    // M = /1  → M = 0
    // N = 8
    // R = /2  → R = 0
    RCC->PLLCFGR =
          RCC_PLLCFGR_PLLSRC_HSI
        | (0 << RCC_PLLCFGR_PLLM_Pos)    // M = /1
        | (8 << RCC_PLLCFGR_PLLN_Pos)    // N = 8
        | (0 << RCC_PLLCFGR_PLLR_Pos)    // R = /2
        | RCC_PLLCFGR_PLLREN;            // Enable R output

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Switch SYSCLK = PLL
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |=  RCC_CFGR_SW_PLL;

    // Wait until switch done
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (3U << RCC_CFGR_SWS_Pos));
}


// =======================================================
// PROTOTYPES
// =======================================================
static void ICA07_InitGPIO(void);
static void ICA07_PartA(void);
static void ICA07_PartB(void);
static void ICA07_PartC(void);


// =======================================================
// MAIN
// =======================================================
int main(void)
{
    ICA07_InitGPIO();     // PA5, PC6, PA8

#ifdef RUN_PART_A
    _USART_Init_USART2(16000000, 115200);
    ICA07_PartA();
#endif

#ifdef RUN_PART_B
    PLL_Init_64MHz();                // ✔ working PLL integrated
    _USART_Init_USART2(64000000, 115200);
    ICA07_PartB();
#endif

#ifdef RUN_PART_C
    PLL_Init_64MHz();
    _USART_Init_USART2(64000000, 115200);
    ICA07_PartC();
#endif

    while (1) __WFI();
}


// =======================================================
// GPIO SETUP
// =======================================================
static void ICA07_InitGPIO(void)
{
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOC);

    GPIO_InitOutput(GPIOA, 5);
    GPIO_InitOutput(GPIOC, 6);

    _GPIO_SetPinMode(GPIOA, 8, _GPIO_PinMode_AlternateFunction);
    GPIOA->AFR[1] &= ~(0xF << 0);
}


// =======================================================
// PART A
// =======================================================
static void ICA07_PartA(void)
{
    
    // Output HSI16 on PA8 with /16 divider
    RCC->CFGR &= ~((7U << 24) | (7U << 28));    // clear MCO bits
    RCC->CFGR |=  (MCO_Sel_HSI16 | MCO_Div16);

    const uint32_t delayCount = 1600000;

    while (1)
    {
        GPIO_Toggle(GPIOA, 5);
        GPIO_Toggle(GPIOC, 6);

        time_ms += 200;

        printf("Tick A - PART A   Count=%lu   Time=%lums\n",
               counter++, time_ms);

        Delay(delayCount);
    }
}


// =======================================================
// PART B — 64 MHz PLL VERIFIED WORKING
// =======================================================
static void ICA07_PartB(void)
{
    Clock_EnableOutput(MCO_Sel_SYSCLK, MCO_Div1);

    const uint32_t delayCount = 1600000;

    while (1)
    {
        GPIO_Toggle(GPIOA, 5);
        GPIO_Toggle(GPIOC, 6);

        time_ms += 25;

        printf("Tick B - PART B   Count=%lu   Time=%lums\n",
               counter++, time_ms);

        Delay(delayCount);
    }
}


// =======================================================
// PART C
// =======================================================
static void ICA07_PartC(void)
{
    // ---- ENABLE LSI FIRST  ----
    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY));   // wait until LSI is ready

    // ---- MCO = LSI / 16 ----
    RCC->CFGR &= ~((7U << 24) | (7U << 28));    // clear MCO bits
    RCC->CFGR |=  (MCO_Sel_LSI | MCO_Div16);

    const uint32_t delayCount = 200000;

    while (1)
    {
        GPIO_Toggle(GPIOA, 5);
        GPIO_Toggle(GPIOC, 6);

        time_ms += 20;

        printf("Tick C - PART C   Count=%lu   Time=%lums\n",
               counter++, time_ms);

        Delay(delayCount);
    }
}

