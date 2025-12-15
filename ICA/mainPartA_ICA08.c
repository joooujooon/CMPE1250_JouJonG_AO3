/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

File    : main.c
Purpose : ICA08 Part A - LED Brightness Control with PWM

*/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "clock.h"
#include "usart.h"
#include "timer.h"

int main(void) 
{
  uint16_t duty = 50;        // Start at 5% duty (50 out of 1000)
  uint8_t direction = 1;     // 1 = increasing, 0 = decreasing
  uint32_t msCounter = 0;
  
  //==================================================================
  // INITIALIZATION
  //==================================================================
  
  // Enable power interface clock
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  
  // Enable GPIO clocks
  _GPIO_ClockEnable(GPIOA);
  _GPIO_ClockEnable(GPIOB);
  _GPIO_ClockEnable(GPIOC);
  
  // Enable TIM14 clock
  RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
  
  // Configure system clock to 40MHz
  FLASH->ACR |= FLASH_ACR_PRFTEN_Msk;
  FLASH->ACR |= FLASH_ACR_LATENCY_1;
  RCC->ICSCR &= ~RCC_ICSCR_HSITRIM_Msk;
  RCC->ICSCR |= 63 << 8;
  Clock_InitPll(PLL_40MHZ);
  
  // Initialize USART for debugging
  _USART_Init_USART2(SystemCoreClock, 115200);
  
  printf("\n========================================\n");
  printf("ICA08 Part A - LED Brightness Control\n");
  printf("========================================\n");
  printf("SYSCLK = %lu Hz\n\n", SystemCoreClock);
  
  //==================================================================
  // GPIO CONFIGURATION - PA4 for TIM14_CH1
  //==================================================================
  
  _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_AlternateFunction);
  _GPIO_SetPinAlternateFunction(GPIOA, 4, 4);  // AF4 = TIM14_CH1
  
  printf("GPIO Configuration:\n");
  printf("  PA4 configured as TIM14_CH1 (AF4)\n\n");
  
  //==================================================================
  // TIMER CONFIGURATION - 1kHz PWM
  //==================================================================
  
  // Timer frequency calculation:
  // SYSCLK = 40 MHz
  // Prescaler = 40 -> Timer clock = 40MHz / 40 = 1 MHz
  // ARR = 1000 -> PWM frequency = 1MHz / 1000 = 1 kHz
  
  Timer_Init(TIM14, 40, 1000);
  Timer_ConfigPWM(TIM14, TIMER_CHANNEL1, TIMER_PWM_MODE1);
  Timer_SetDuty(TIM14, TIMER_CHANNEL1, duty);
  Timer_EnableOutput(TIM14, TIMER_CHANNEL1);
  Timer_Start(TIM14);
  
  printf("Timer Configuration:\n");
  printf("  PWM Frequency: 1 kHz\n");
  printf("  Prescaler: 40 (Timer clock = 1 MHz)\n");
  printf("  ARR: 1000\n");
  printf("  Initial Duty: 5%% (%u/1000)\n\n", duty);
  
  printf("LED will cycle brightness from 5%% to 95%% over 1 second\n");
  printf("Observe LED on PA4\n");
  printf("Use oscilloscope to verify PWM signal\n\n");
  
  //==================================================================
  // MAIN LOOP
  //==================================================================
  
  while(1)
  {
    // Check timer update flag (occurs every 1ms at 1kHz)
    if(Timer_CheckUpdateFlag(TIM14))
    {
      Timer_ClearUpdateFlag(TIM14);
      msCounter++;
      
      // Update duty cycle every 1ms for smooth transition
      // Total range: 50 to 950 (5% to 95%)
      // Time: 900 steps = 900ms per direction
      
      if(direction)  // Increasing brightness
      {
        duty++;
        if(duty >= 950)  // Reached 95%
        {
          direction = 0;
          printf("Peak brightness reached (95%%)\n");
        }
      }
      else  // Decreasing brightness
      {
        duty--;
        if(duty <= 50)   // Reached 5%
        {
          direction = 1;
          printf("Minimum brightness reached (5%%)\n");
        }
      }
      
      Timer_SetDuty(TIM14, TIMER_CHANNEL1, duty);
      
      // Print status every second
      if(msCounter >= 1000)
      {
        msCounter = 0;
        printf("Current duty: %u/1000 (%u%%)\n", duty, (duty * 100) / 1000);
      }
    }
  }
}

/*************************** End of file ****************************/