// GPIO Library Header (Template Version 1.0)
//
// <gpio.h>
//
// AUTHOR: Jou Jon Galenzoga
//
// Version History
// Created September 25, 2025, CREATE NOTES
// Updated December 2025, Added Alternate Function support for ICA08
// 
///////////////////////////////////////////////////////////////////////

#ifndef GPIO_LIB_H
#define GPIO_LIB_H

#include "stm32g031xx.h"
#include <stdint.h>

/***** Basic Prototypes *****/

/**
 * @brief  Set GPIO pin as output
 * @param  GPIO Port
 * @param  GPIO pin
 */
void GPIO_InitOutput(GPIO_TypeDef*, uint16_t);

/**
 * @brief  Set GPIO pin as input
 * @param  GPIO Port
 * @param  GPIO pin
 */
void GPIO_InitInput(GPIO_TypeDef*, uint16_t);

/**
 * @brief  Set bit in Port
 * @param  Port
 * @param  pin
 */
void GPIO_Set(GPIO_TypeDef*, uint16_t);

/**
 * @brief  Clear bit in Port
 * @param  Port
 * @param  pin
 */
void GPIO_Clear(GPIO_TypeDef*, uint16_t);

/**
 * @brief  Toggle bit in Port
 * @param  Port
 * @param  pin
 */
void GPIO_Toggle(GPIO_TypeDef*, uint16_t);

/**
 * @brief  Reads pin in Port
 * @param  Port
 * @param  pin
 */
int GPIO_Read(GPIO_TypeDef*, uint16_t);

//======================================================================
// Pin Modes (matching MODER bit patterns)
//======================================================================
typedef enum
{
    _GPIO_PinMode_Input = 0x00,              // digital input
    _GPIO_PinMode_Output = 0x01,             // digital output
    _GPIO_PinMode_AlternateFunction = 0x02,  // alternate function
    _GPIO_PinMode_Analog = 0x03              // analog mode (default)
} _GPIO_PinMode;

//======================================================================
// Output Type
//======================================================================
typedef enum
{
    _GPIO_OutputType_PushPull = 0,   // Push-pull output
    _GPIO_OutputType_OpenDrain = 1   // Open-drain output
} _GPIO_OutputType;

//======================================================================
// Output Speed
//======================================================================
typedef enum
{
    _GPIO_Speed_Low = 0,         // Low speed
    _GPIO_Speed_Medium = 1,      // Medium speed
    _GPIO_Speed_High = 2,        // High speed
    _GPIO_Speed_VeryHigh = 3     // Very high speed
} _GPIO_Speed;

//======================================================================
// Pull-up/Pull-down
//======================================================================
typedef enum
{
    _GPIO_Pull_None = 0,     // No pull-up or pull-down
    _GPIO_Pull_Up = 1,       // Pull-up
    _GPIO_Pull_Down = 2      // Pull-down
} _GPIO_Pull;

//======================================================================
// Extended Functions
//======================================================================

/**
 * @brief Enable clock for GPIO port
 */
void _GPIO_ClockEnable(GPIO_TypeDef *pPort);

/**
 * @brief Toggle output pin
 */
void _GPIO_PinToggle(GPIO_TypeDef *pPort, int PinNumber);

/**
 * @brief Set output pin high
 */
void _GPIO_PinSet(GPIO_TypeDef *pPort, int PinNumber);

/**
 * @brief Clear output pin (set low)
 */
void _GPIO_PinClear(GPIO_TypeDef *pPort, int PinNumber);

/**
 * @brief Set pin mode (Input, Output, Alternate Function, Analog)
 */
void _GPIO_SetPinMode(GPIO_TypeDef *pPort, int PinNumber, _GPIO_PinMode mode);

/**
 * @brief Read input pin state (IDR register)
 */
int _GPIO_GetPinIState(GPIO_TypeDef *pPort, int PinNumber);

/**
 * @brief Read output pin state (ODR register)
 */
int _GPIO_GetPinOState(GPIO_TypeDef *pPort, int PinNumber);

//======================================================================
// Advanced Functions (ICA08+)
//======================================================================

/**
 * @brief Set GPIO pin alternate function
 * @param pPort GPIO Port
 * @param PinNumber Pin number (0-15)
 * @param AF Alternate function number (0-7)
 * 
 * Common AF mappings for STM32G031:
 *   AF0: TIM14_CH1 on PB1
 *   AF1: USART1, USART2
 *   AF2: TIM1, TIM3
 *   AF4: TIM14_CH1 on PA4, PA7
 *   AF5: TIM16, TIM17
 */
void _GPIO_SetPinAlternateFunction(GPIO_TypeDef *pPort, int PinNumber, uint8_t AF);

/**
 * @brief Set output type (push-pull or open-drain)
 */
void _GPIO_SetOutputType(GPIO_TypeDef *pPort, int PinNumber, _GPIO_OutputType outputType);

/**
 * @brief Set output speed
 */
void _GPIO_SetSpeed(GPIO_TypeDef *pPort, int PinNumber, _GPIO_Speed speed);

/**
 * @brief Set pull-up/pull-down resistor
 */
void _GPIO_SetPull(GPIO_TypeDef *pPort, int PinNumber, _GPIO_Pull pull);

#endif