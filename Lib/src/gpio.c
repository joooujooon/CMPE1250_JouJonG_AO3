///////////////////////////////////////////////////////////////////////// 
//
//  GPIO LIBRARY
//
//  AUTHOR: Jou Jon Galenzoga
//  FILe: gpio.c
//  Version History
//  Created September 25, 2025, CREATE NOTES
// 
///////////////////////////////////////////////////////////////////////
#include "stm32g031xx.h"
#include "gpio.h"

//======================================================
// Initialize pin as INPUT
//======================================================
void GPIO_InitInput(GPIO_TypeDef* gpio, uint16_t pin)
{
    gpio->MODER &= ~(0x3U << (pin * 2));      // 00 = input
}

void GPIO_InitOutput(GPIO_TypeDef* gpio, uint16_t pin)
{
    gpio->MODER &= ~(0x3U << (pin * 2));      // clear bits
    gpio->MODER |=  (0x1U << (pin * 2));      // 01 = output
}

void GPIO_Set(GPIO_TypeDef* gpio, uint16_t pin)
{
    gpio->BSRR = (1U << pin);                 // set bit
}

void GPIO_Clear(GPIO_TypeDef* gpio, uint16_t pin)
{
    gpio->BSRR = (1U << (pin + 16));          // reset bit (upper half)
}

void GPIO_Toggle(GPIO_TypeDef* gpio, uint16_t pin)
{
    gpio->ODR ^= (1U << pin);                 // XOR to flip bit
}

int GPIO_Read(GPIO_TypeDef* gpio, uint16_t pin)
{
    return ( (gpio->IDR & (1U << pin)) ? 1 : 0 );
}

//======================================================
// ICA04
//======================================================

void _GPIO_ClockEnable(GPIO_TypeDef *pPort)
{
    if (pPort == GPIOA) RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    else if (pPort == GPIOB) RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    else if (pPort == GPIOC) RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
}

void _GPIO_PinToggle(GPIO_TypeDef *pPort, int PinNumber)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    pPort->ODR ^= (1U << PinNumber);
}

void _GPIO_PinSet(GPIO_TypeDef *pPort, int PinNumber)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    pPort->BSRR = (1U << PinNumber);
}

void _GPIO_PinClear(GPIO_TypeDef *pPort, int PinNumber)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    pPort->BSRR = (1U << (PinNumber + 16));
}

void _GPIO_SetPinMode(GPIO_TypeDef *pPort, int PinNumber, _GPIO_PinMode mode)
{
    if (PinNumber < 0 || PinNumber > 15) return;

    pPort->MODER &= ~(0x3U << (PinNumber * 2));  
    pPort->MODER |= ((uint32_t)mode << (PinNumber * 2));
}

int _GPIO_GetPinIState(GPIO_TypeDef *pPort, int PinNumber)
{
    if (PinNumber < 0 || PinNumber > 15) return 0;
    return (pPort->IDR >> PinNumber) & 1U;
}

int _GPIO_GetPinOState(GPIO_TypeDef *pPort, int PinNumber)
{
    if (PinNumber < 0 || PinNumber > 15) return 0;
    return (pPort->ODR >> PinNumber) & 1U;
}