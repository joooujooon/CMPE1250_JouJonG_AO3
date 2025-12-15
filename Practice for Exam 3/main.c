/*********************************************************************
* Part A TEST
* - Use your Clock_InitPll(...) to set SYSCLK (PLL)
* - TIM16 generates update event every 20 ms (poll UIF)
* - Toggle a GPIO pin on each update event
*
* Expected scope result on PB3:
*   Toggle every 20 ms -> full cycle 40 ms -> 25 Hz square wave
*********************************************************************/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "clock.h"
#include "usart.h"

// Scope test pin
#define TEST_PORT GPIOB
#define TEST_PIN  3

static void TIM16_20ms_Init(void)
{
    // Enable TIM16 clock
    RCC->APBENR2 |= RCC_APBENR2_TIM16EN;

    // Disable timer while configuring
    TIM16->CR1 &= ~TIM_CR1_CEN;

    // Make timer tick = 10 kHz (0.1 ms per tick)
    // PSC = (SystemCoreClock / 10,000) - 1
    TIM16->PSC = (uint16_t)((SystemCoreClock / 10000U) - 1U);

    // 20 ms / 0.1 ms = 200 ticks => ARR = 199
    TIM16->ARR = 199U;

    // Force update event to load PSC/ARR
    TIM16->EGR |= TIM_EGR_UG;

    // Clear UIF (rc_w0 safe clear)
    TIM16->SR = ~TIM_SR_UIF;

    // Enable counter
    TIM16->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
    // -------------------- Init clocks/power/flash like prof demo --------------------
    RCC->APBENR1 |= RCC_APBENR1_PWREN;

    FLASH->ACR |= FLASH_ACR_PRFTEN_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_1;

    // Optional HSI trim (prof demo style)
    RCC->ICSCR &= ~RCC_ICSCR_HSITRIM_Msk;
    RCC->ICSCR |= (63U << 8);

    // Use your existing clock library PLL config
    // If your library has PLL_50MHZ, use that. Otherwise keep PLL_40MHZ.
    Clock_InitPll(PLL_40MHZ);
    //Clock_InitPll(PLL_50MHZ);

    printf("SYSCLK = %lu Hz\r\n", SystemCoreClock);

    // -------------------- GPIO for scope toggle --------------------
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    _GPIO_SetPinMode(TEST_PORT, TEST_PIN, _GPIO_PinMode_Output);

    // -------------------- TIM16 20ms tick --------------------
    TIM16_20ms_Init();

    // -------------------- Main loop: poll UIF and toggle --------------------
    while (1)
    {
        if (TIM16->SR & TIM_SR_UIF)
        {
            TIM16->SR = ~TIM_SR_UIF;           // clear update flag
            _GPIO_PinToggle(TEST_PORT, TEST_PIN); // toggle output pin
        }
    }
}
