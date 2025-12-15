/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

File    : main.c
Purpose : ICA08 Part B - Budget DAC using PWM + RC Filter

*/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "clock.h"
#include "usart.h"
#include "timer.h"

int main(void) 
{
  //==================================================================
  // INITIALIZATION
  //==================================================================
  
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  
  _GPIO_ClockEnable(GPIOA);
  _GPIO_ClockEnable(GPIOB);
  _GPIO_ClockEnable(GPIOC);
  
  RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
  
  // Configure system clock to 40MHz
  FLASH->ACR |= FLASH_ACR_PRFTEN_Msk;
  FLASH->ACR |= FLASH_ACR_LATENCY_1;
  RCC->ICSCR &= ~RCC_ICSCR_HSITRIM_Msk;
  RCC->ICSCR |= 63 << 8;
  Clock_InitPll(PLL_40MHZ);
  
  _USART_Init_USART2(SystemCoreClock, 115200);
  
  printf("\n========================================\n");
  printf("ICA08 Part B - Budget DAC\n");
  printf("========================================\n");
  printf("SYSCLK = %lu Hz\n\n", SystemCoreClock);
  
  //==================================================================
  // GPIO CONFIGURATION - PA7 for TIM14_CH1
  //==================================================================
  
  _GPIO_SetPinMode(GPIOA, 7, _GPIO_PinMode_AlternateFunction);
  _GPIO_SetPinAlternateFunction(GPIOA, 7, 4);  // AF4 = TIM14_CH1
  
  printf("GPIO Configuration:\n");
  printf("  PA7 configured as TIM14_CH1 (AF4)\n\n");
  
  //==================================================================
  // TIMER CONFIGURATION - 2kHz PWM with Maximum Resolution
  //==================================================================
  
  // For maximum resolution, minimize prescaler
  // SYSCLK = 40 MHz
  // Prescaler = 1 -> Timer clock = 40 MHz
  // For 2 kHz: ARR = 40MHz / 2kHz = 20,000
  // This gives us 20,000 steps of duty cycle control!
  
  Timer_Init(TIM14, 1, 20000);
  Timer_ConfigPWM(TIM14, TIMER_CHANNEL1, TIMER_PWM_MODE1);
  Timer_SetDuty(TIM14, TIMER_CHANNEL1, 10000);  // 50% duty initially
  Timer_EnableOutput(TIM14, TIMER_CHANNEL1);
  Timer_Start(TIM14);
  
  printf("Timer Configuration:\n");
  printf("  PWM Frequency: 2 kHz\n");
  printf("  Prescaler: 1 (Timer clock = 40 MHz)\n");
  printf("  ARR: 20000 (maximum resolution)\n");
  printf("  Initial Duty: 50%% (10000/20000)\n\n");
  
  //==================================================================
  // RC FILTER INFORMATION
  //==================================================================
  
  printf("RC Filter:\n");
  printf("  R = 10 kOhm\n");
  printf("  C = 100 nF\n");
  printf("  Cutoff frequency = 159 Hz\n");
  printf("  (PWM freq 2kHz >> 159Hz, good filtering)\n\n");
  
  //==================================================================
  // TEST INSTRUCTIONS
  //==================================================================
  
  printf("========================================\n");
  printf("MEASUREMENT INSTRUCTIONS\n");
  printf("========================================\n\n");
  
  printf("Test Point Locations:\n");
  printf("  TP1: PA7 pin (PWM output before resistor)\n");
  printf("  TP2: Between 10k resistor and 100nF cap\n");
  printf("  TP3: Same as TP2 (measure DC voltage here)\n\n");
  
  printf("Current Configuration: 50%% Duty\n");
  printf("  CCR1 = 10000\n");
  printf("  Expected voltage at TP3: 1.65V\n\n");
  
  printf("========================================\n");
  printf("TEST SEQUENCE\n");
  printf("========================================\n\n");
  
  printf("Step 1: Verify PWM at TP1 with oscilloscope\n");
  printf("  - Should see 2kHz square wave\n");
  printf("  - Amplitude: 0V to 3.3V\n");
  printf("  - Current duty: 50%%\n\n");
  
  printf("Step 2: Measure DC voltage at TP3 with DMM\n");
  printf("  - Current setting: 50%% duty\n");
  printf("  - Expected: 1.65V\n");
  printf("  - Measured: _______V\n\n");
  
  printf("========================================\n");
  printf("TO TEST OTHER DUTY CYCLES:\n");
  printf("========================================\n\n");
  
  printf("Modify code and recompile for each test:\n\n");
  
  printf("Duty   |  CCR1 Value  | Expected Voltage\n");
  printf("-------|--------------|------------------\n");
  printf("  0%%   |      0       |     0.00V\n");
  printf("  5%%   |   1,000      |     0.17V\n");
  printf(" 25%%   |   5,000      |     0.83V\n");
  printf(" 50%%   |  10,000      |     1.65V  <-- Current\n");
  printf(" 75%%   |  15,000      |     2.48V\n");
  printf("100%%   |  20,000      |     3.30V\n\n");
  
  printf("To change duty, modify this line in code:\n");
  printf("  Timer_SetDuty(TIM14, TIMER_CHANNEL1, XXXXX);\n\n");
  
  printf("Formula: Vout = 3.3V × (Duty %% / 100)\n\n");
  
  //==================================================================
  // MAIN LOOP - Static output
  //==================================================================
  
  while(1)
  {
    // No action needed - PWM runs continuously
    // Change Timer_SetDuty() value above to test different voltages
  }
}

/*************************** End of file ****************************/
