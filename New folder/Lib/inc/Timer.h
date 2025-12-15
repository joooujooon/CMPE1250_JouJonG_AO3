/////////////////////////////////////////////////////////////////////////
//
//  TIMER LIBRARY (ICA08)
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   timer.h
//  Version History
//    ICA08 - Basic Timer and PWM functions
//
/////////////////////////////////////////////////////////////////////////

#ifndef TIMER_LIB_H
#define TIMER_LIB_H

#include "stm32g031xx.h"
#include <stdint.h>

// =====================================================================
// PWM Mode Selection
// =====================================================================
typedef enum
{
    TIMER_PWM_MODE1 = 0,  // Active when CNT < CCR (normal)
    TIMER_PWM_MODE2 = 1   // Active when CNT > CCR (inverted)
} Timer_PWMMode;

// =====================================================================
// Timer Channel Selection
// =====================================================================
typedef enum
{
    TIMER_CHANNEL1 = 1
} Timer_Channel;

// =====================================================================
// Basic Timer Functions
// =====================================================================

/**
 * @brief Initialize timer with prescaler and period
 * @param pTimer Timer instance (TIM14, TIM16, etc.)
 * @param prescaler Prescaler value (actual value, function handles -1)
 * @param period Period value (ARR, actual value, function handles -1)
 */
void Timer_Init(TIM_TypeDef *pTimer, uint16_t prescaler, uint16_t period);

/**
 * @brief Start the timer counter
 */
void Timer_Start(TIM_TypeDef *pTimer);

/**
 * @brief Stop the timer counter
 */
void Timer_Stop(TIM_TypeDef *pTimer);

/**
 * @brief Check if Update Interrupt Flag is set
 * @return 1 if flag set, 0 otherwise
 */
int Timer_CheckUpdateFlag(TIM_TypeDef *pTimer);

/**
 * @brief Clear Update Interrupt Flag
 */
void Timer_ClearUpdateFlag(TIM_TypeDef *pTimer);

// =====================================================================
// PWM Functions
// =====================================================================

/**
 * @brief Configure timer channel for PWM output
 * @param pTimer Timer instance
 * @param channel Channel number (use TIMER_CHANNEL1)
 * @param mode PWM mode (TIMER_PWM_MODE1 or TIMER_PWM_MODE2)
 */
void Timer_ConfigPWM(TIM_TypeDef *pTimer, Timer_Channel channel, Timer_PWMMode mode);

/**
 * @brief Set PWM duty cycle (raw value)
 * @param pTimer Timer instance
 * @param channel Channel number
 * @param duty Duty cycle value (0 to ARR)
 */
void Timer_SetDuty(TIM_TypeDef *pTimer, Timer_Channel channel, uint16_t duty);

/**
 * @brief Set PWM duty cycle as percentage
 * @param pTimer Timer instance
 * @param channel Channel number
 * @param percent Duty cycle percentage (0-100)
 */
void Timer_SetDutyPercent(TIM_TypeDef *pTimer, Timer_Channel channel, uint8_t percent);

/**
 * @brief Enable PWM output on channel
 */
void Timer_EnableOutput(TIM_TypeDef *pTimer, Timer_Channel channel);

/**
 * @brief Disable PWM output on channel
 */
void Timer_DisableOutput(TIM_TypeDef *pTimer, Timer_Channel channel);

/**
 * @brief Get current ARR value
 * @return Current auto-reload register value
 */
uint16_t Timer_GetARR(TIM_TypeDef *pTimer);

#endif // TIMER_LIB_H