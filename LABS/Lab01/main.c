/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include "stm32g031xx.h"
#include "gpio.h"
#include "usart.h"

//==================================================
// STRUCT FOR FLEXIBLE PIN ASSIGNMENTS
//==================================================
typedef struct
{
    int pin_number;
    GPIO_TypeDef *pin_port;
} PIN_ASSIGNMENT;

//==================================================
// PIN ASSIGNMENTS 
//==================================================
const PIN_ASSIGNMENT LED_A    = { 6,  GPIOA };   // D1
const PIN_ASSIGNMENT LED_B    = { 11, GPIOA };   // D2
const PIN_ASSIGNMENT SWITCH_A = { 0,  GPIOB };   // S1
const PIN_ASSIGNMENT SWITCH_B = { 2,  GPIOB };   // S2

//==================================================
// UI CONSTANTS (EASY TO MOVE AROUND)
//==================================================
#define UI_SWITCH_ROW   1
#define UI_LED_ROW      2
#define UI_SW1_COL      17
#define UI_SW2_COL      20
#define UI_LED1_COL     17
#define UI_LED2_COL     20

//==================================================
// FUNCTION PROTOTYPES
//==================================================
void InitHardware(void);
void RenderStaticUI(void);
void RenderSwitchState(int col, int row, const PIN_ASSIGNMENT *sw);
void RenderLEDState(int col, int row, const PIN_ASSIGNMENT *led);

//==================================================
// HARDWARE INITIALIZATION
//==================================================
void InitHardware(void)
{
    // GPIO
    GPIO_SetPinMode(LED_A.pin_port, LED_A.pin_number,
                    GPIO_PinMode_Output, GPIO_Clock_On);
    GPIO_SetPinMode(LED_B.pin_port, LED_B.pin_number,
                    GPIO_PinMode_Output, GPIO_Clock_On);

    GPIO_SetPinMode(SWITCH_A.pin_port, SWITCH_A.pin_number,
                    GPIO_PinMode_Input, GPIO_Clock_On);
    GPIO_SetPinMode(SWITCH_B.pin_port, SWITCH_B.pin_number,
                    GPIO_PinMode_Input, GPIO_Clock_On);

    // UART
    USART_Init(38400); // or USART2_Init(38400)
}

//==================================================
// STATIC UI
//==================================================
void RenderStaticUI(void)
{
    USART_ClearScreen();
    USART_SetCursor(1, 1);
    USART_WriteString("Switch States : [ ] [ ]");

    USART_SetCursor(2, 1);
    USART_WriteString("LED Control   : [ ] [ ]");

    USART_SetCursor(4, 1);
    USART_WriteString("Press 1 or 2 to toggle LEDs");
}

//==================================================
// SWITCH STATE RENDER
//==================================================
void RenderSwitchState(int col, int row, const PIN_ASSIGNMENT *sw)
{
    uint8_t raw = GPIO_ReadPin(sw->pin_port, sw->pin_number);
    uint8_t pressed = (raw == 0);   // ACTIVE LOW

    USART_SetCursor(row, col);
    USART_WriteChar(pressed ? 'X' : ' ');
}

//==================================================
// LED STATE RENDER (ACTIVE LOW)
//==================================================
void RenderLEDState(int col, int row, const PIN_ASSIGNMENT *led)
{
    uint8_t raw = GPIO_ReadPin(led->pin_port, led->pin_number);
    uint8_t on = (raw == 0);

    USART_SetCursor(row, col);
    USART_WriteChar(on ? 'X' : ' ');
}

//==================================================
// MAIN PROGRAM — UNCOMMENT AS YOU PROGRESS
//==================================================
int main(void)
{
    InitHardware();

    /*********************************************************
     * PART A — UART TEST ONLY
     * TEST: You should see "UART OK" in TeraTerm.
     *********************************************************/
    /*
    USART_WriteString("UART OK\n");
    while (1) { }
    */


    /*********************************************************
     * PART B — STATIC UI
     * TEST: Should print the three UI lines ONCE.
     *********************************************************/
    /*
    RenderStaticUI();
    while (1) { }
    */


    /*********************************************************
     * PART C — SWITCH TEST
     * TEST: Press S1/S2 → X appears in SWITCH line
     *********************************************************/
    /*
    RenderStaticUI();
    while (1)
    {
        RenderSwitchState(UI_SW1_COL, UI_SWITCH_ROW, &SWITCH_A);
        RenderSwitchState(UI_SW2_COL, UI_SWITCH_ROW, &SWITCH_B);
    }
    */


    /*********************************************************
     * PART D — LED STATE TEST
     * TEST: Manually toggle LED_A with GPIO_WritePinLow()
     *       Check LED line updates correctly.
     *********************************************************/
    /*
    RenderStaticUI();
    while (1)
    {
        RenderLEDState(UI_LED1_COL, UI_LED_ROW, &LED_A);
        RenderLEDState(UI_LED2_COL, UI_LED_ROW, &LED_B);
    }
    */
