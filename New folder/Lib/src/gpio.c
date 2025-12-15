///////////////////////////////////////////////////////////////////////// 
//
//  GPIO LIBRARY
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   gpio.c
//  Version History
//    Created September 25, 2025
//    Updated for ICA08 - Added Alternate Function support
// 
///////////////////////////////////////////////////////////////////////

#include "stm32g031xx.h"
#include "gpio.h"

//======================================================================
// Basic GPIO Functions
//======================================================================

void GPIO_InitInput(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return;
    gpio->MODER &= ~(0x3U << (pin * 2));      // 00 = input
}

void GPIO_InitOutput(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return;
    gpio->MODER &= ~(0x3U << (pin * 2));      // Clear bits
    gpio->MODER |=  (0x1U << (pin * 2));      // 01 = output
}

void GPIO_Set(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return;
    gpio->BSRR = (1U << pin);                 // Set bit
}

void GPIO_Clear(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return;
    gpio->BSRR = (1U << (pin + 16));          // Reset bit (upper half)
}

void GPIO_Toggle(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return;
    gpio->ODR ^= (1U << pin);                 // XOR to flip bit
}

int GPIO_Read(GPIO_TypeDef* gpio, uint16_t pin)
{
    if (pin > 15) return 0;
    return ((gpio->IDR & (1U << pin)) ? 1 : 0);
}

//======================================================================
// Extended GPIO Functions (ICA04+)
//======================================================================

void _GPIO_ClockEnable(GPIO_TypeDef *pPort)
{
    if (pPort == GPIOA)
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    else if (pPort == GPIOB)
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    else if (pPort == GPIOC)
        RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
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

//======================================================================
// Advanced GPIO Functions (ICA08+)
//======================================================================

void _GPIO_SetPinAlternateFunction(GPIO_TypeDef *pPort, int PinNumber, uint8_t AF)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    if (AF > 7) return;  // AF can only be 0-7
    
    // Determine which AFR register to use
    // AFR[0] = AFRL for pins 0-7
    // AFR[1] = AFRH for pins 8-15
    uint8_t regIndex = (PinNumber < 8) ? 0 : 1;
    uint8_t bitPos = (PinNumber % 8) * 4;  // Each pin uses 4 bits
    
    // Clear the 4 bits for this pin
    pPort->AFR[regIndex] &= ~(0xFU << bitPos);
    
    // Set the alternate function
    pPort->AFR[regIndex] |= ((uint32_t)AF << bitPos);
}

void _GPIO_SetOutputType(GPIO_TypeDef *pPort, int PinNumber, _GPIO_OutputType outputType)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    
    if (outputType == _GPIO_OutputType_OpenDrain)
        pPort->OTYPER |= (1U << PinNumber);
    else
        pPort->OTYPER &= ~(1U << PinNumber);
}

void _GPIO_SetSpeed(GPIO_TypeDef *pPort, int PinNumber, _GPIO_Speed speed)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    
    // Clear speed bits (2 bits per pin)
    pPort->OSPEEDR &= ~(0x3U << (PinNumber * 2));
    
    // Set speed
    pPort->OSPEEDR |= ((uint32_t)speed << (PinNumber * 2));
}

void _GPIO_SetPull(GPIO_TypeDef *pPort, int PinNumber, _GPIO_Pull pull)
{
    if (PinNumber < 0 || PinNumber > 15) return;
    
    // Clear pull bits (2 bits per pin)
    pPort->PUPDR &= ~(0x3U << (PinNumber * 2));
    
    // Set pull configuration
    pPort->PUPDR |= ((uint32_t)pull << (PinNumber * 2));
}