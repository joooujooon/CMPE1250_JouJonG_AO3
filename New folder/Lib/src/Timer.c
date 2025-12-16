/////////////////////////////////////////////////////////////////////////
//
//  TIMER LIBRARY (ICA08)
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   timer.c
//  Version History
//    ICA08 - Basic Timer and PWM functions
//
/////////////////////////////////////////////////////////////////////////

#include <Timer.h>

// =====================================================================
// Basic Timer Functions
// =====================================================================

void Timer_Init(TIM_TypeDef *pTimer, uint16_t prescaler, uint16_t period)
{
    // Disable timer during configuration
    pTimer->CR1 &= ~TIM_CR1_CEN;
    
    // Set prescaler and period (subtract 1 as they are 0-indexed)
    pTimer->PSC = prescaler - 1;
    pTimer->ARR = period - 1;
    
    // Generate update event to load prescaler
    pTimer->EGR |= TIM_EGR_UG;
    
    // Clear update flag
    pTimer->SR &= ~TIM_SR_UIF;
}

void Timer_Start(TIM_TypeDef *pTimer)
{
    pTimer->CR1 |= TIM_CR1_CEN;
}

void Timer_Stop(TIM_TypeDef *pTimer)
{
    pTimer->CR1 &= ~TIM_CR1_CEN;
}

int Timer_CheckUpdateFlag(TIM_TypeDef *pTimer)
{
    return (pTimer->SR & TIM_SR_UIF) ? 1 : 0;
}

void Timer_ClearUpdateFlag(TIM_TypeDef *pTimer)
{
    pTimer->SR &= ~TIM_SR_UIF;
}

uint16_t Timer_GetARR(TIM_TypeDef *pTimer)
{
    return pTimer->ARR;
}

// =====================================================================
// PWM Functions
// =====================================================================

void Timer_ConfigPWM(TIM_TypeDef *pTimer, Timer_Channel channel, Timer_PWMMode mode)
{
    // For TIM14, only channel 1 exists
    // Clear output compare mode bits (OC1M in CCMR1)
    pTimer->CCMR1 &= ~TIM_CCMR1_OC1M;
    
    // Set PWM mode
    // PWM Mode 1: 110 binary (active when CNT < CCR)
    // PWM Mode 2: 111 binary (active when CNT > CCR)
    if (mode == TIMER_PWM_MODE1)
    {
        pTimer->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;  // 0b110
    }
    else // TIMER_PWM_MODE2
    {
        pTimer->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;  // 0b111
    }
    
    // Enable preload for smooth transitions
    pTimer->CCMR1 |= TIM_CCMR1_OC1PE;
}

void Timer_SetDuty(TIM_TypeDef *pTimer, Timer_Channel channel, uint16_t duty)
{
    // For TIM14 CH1
    pTimer->CCR1 = duty;
}

void Timer_SetDutyPercent(TIM_TypeDef *pTimer, Timer_Channel channel, uint8_t percent)
{
    if (percent > 100)
        percent = 100;
    
    uint32_t arr = pTimer->ARR + 1;  // ARR is 0-indexed
    uint16_t duty = (arr * percent) / 100;
    
    Timer_SetDuty(pTimer, channel, duty);
}

void Timer_EnableOutput(TIM_TypeDef *pTimer, Timer_Channel channel)
{
    // Enable capture/compare output for channel 1
    pTimer->CCER |= TIM_CCER_CC1E;
}

void Timer_DisableOutput(TIM_TypeDef *pTimer, Timer_Channel channel)
{
    pTimer->CCER &= ~TIM_CCER_CC1E;
}