// GPIO Library Header (Template Version 1.0)
//
// <gpio.h>
//
// AUTHOR: Jou Jon Galenzoga
//
// Version History
// Created September 25, 2025, CREATE NOTES
// 
///////////////////////////////////////////////////////////////////////

#ifndef GPIO_LIB_H
#define GPIO_LIB_H

#include "stm32g031xx.h"
#include <stdint.h>

/***** Prototypes *****/

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

// Pin Modes (matching MODER bit patterns)
typedef enum
{
    _GPIO_PinMode_Input = 0x00,              // digital input
    _GPIO_PinMode_Output = 0x01,             // digital output
    _GPIO_PinMode_AlternateFunction = 0x02,  // alternate function
    _GPIO_PinMode_Analog = 0x03              // analog mode (default)
} _GPIO_PinMode;

// Clock enable (optional)
void _GPIO_ClockEnable(GPIO_TypeDef *pPort);

// Output pin actions
void _GPIO_PinToggle(GPIO_TypeDef *pPort, int PinNumber);
void _GPIO_PinSet(GPIO_TypeDef *pPort, int PinNumber);
void _GPIO_PinClear(GPIO_TypeDef *pPort, int PinNumber);

// Pin mode
void _GPIO_SetPinMode(GPIO_TypeDef *pPort, int PinNumber, _GPIO_PinMode mode);

// Read input and output states
int _GPIO_GetPinIState(GPIO_TypeDef *pPort, int PinNumber);
int _GPIO_GetPinOState(GPIO_TypeDef *pPort, int PinNumber);


#endif