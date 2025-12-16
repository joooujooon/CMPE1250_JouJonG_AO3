/*********************************************************************************************     // file header
* File    : main.c                                                                                // file name
* Purpose : COMBINED TEMPLATE (LAB01 Parts A–G + ICA07 Parts A–C)                                  // purpose
* Author  : Your Name                                                                             // author
*********************************************************************************************/ // end header

#include <stdio.h>                                                                                 // sprintf/snprintf/printf formatting
#include <stdint.h>                                                                                // fixed width integer types
#include "stm32g031xx.h"                                                                           // STM32G031 register definitions
#include "gpio.h"                                                                                  // GPIO helper library (yours)
#include "usart.h"                                                                                 // USART helper library (yours)
#include "clock.h"                                                                                 // clock/PLL/MCO library (ICA07)
#include "timer.h"                                                                                 // timer library (for accurate delays)

//================================================================================================// divider
// PART SELECTOR (ENABLE ONLY ONE PART TOTAL)                                                      // instruction
//================================================================================================// divider
//#define RUN_LAB01_PART_A                                                                          // compile Lab01 Part A
//#define RUN_LAB01_PART_B                                                                          // compile Lab01 Part B
//#define RUN_LAB01_PART_C                                                                          // compile Lab01 Part C
//#define RUN_LAB01_PART_D                                                                          // compile Lab01 Part D
//#define RUN_LAB01_PART_E                                                                          // compile Lab01 Part E
//#define RUN_LAB01_PART_F                                                                          // compile Lab01 Part F
//#define RUN_LAB01_PART_G                                                                          // compile Lab01 Part G
//#define RUN_ICA07_PART_A                                                                          // compile ICA07 Part A
//#define RUN_ICA07_PART_B                                                                            // compile ICA07 Part B
//#define RUN_ICA07_PART_C                                                                          // compile ICA07 Part C
//#define  RUN_ICA07_PART_D
//#define  RUN_ICA07_PART_E


//================================================================================================// divider
// TERMINAL SIZE (ANSI)                                                                            // terminal sizing
//================================================================================================// divider
#define TERM_ROWS   24                                                                             // assumed terminal rows
#define TERM_COLS   80                                                                             // assumed terminal columns

//================================================================================================// divider
// BASIC DELAY (BUSY WAIT)                                                                         // simple delay
//================================================================================================// divider
static void Delay(volatile uint32_t d)                                                             // delay function prototype
{                                                                                                  // start delay function
    while (d--) { }                                                                                // waste CPU cycles
}                                                                                                  // end delay function

//================================================================================================// divider
// PIN ASSIGNMENT STRUCT (GENERIC TEMPLATE)                                                        // mapping struct
//================================================================================================// divider
typedef struct                                                                                      // define struct
{                                                                                                  // start struct
    int pin;                                                                                        // pin number 0..15
    GPIO_TypeDef *port;                                                                             // port pointer GPIOA/B/C
} PIN_ASSIGNMENT;                                                                                   // struct name

//================================================================================================// divider
// GENERIC PIN MAP (EDIT TO MATCH YOUR BOARD)                                                      // pin constants
//================================================================================================// divider
static const PIN_ASSIGNMENT LED_A    = { 0,  GPIOB };                                               // LED A pin (example)
static const PIN_ASSIGNMENT LED_B    = { 2,  GPIOB };                                               // LED B pin (example)
static const PIN_ASSIGNMENT SWITCH_A = { 10, GPIOA };                                               // Switch A pin (example)
static const PIN_ASSIGNMENT SWITCH_B = { 1,  GPIOB };                                               // Switch B pin (example)

//================================================================================================// divider
// ICA07 PIN MAP (USED IN ICA07 PARTS)                                                              // ICA07 pins
//================================================================================================// divider
static const PIN_ASSIGNMENT PIN_PA0 = { 0, GPIOA };                                                 // PA0 output (Part B requirement)
static const PIN_ASSIGNMENT PIN_PA5 = { 5, GPIOA };                                                 // PA5 output (scope/LED)
static const PIN_ASSIGNMENT PIN_PC6 = { 6, GPIOC };                                                 // PC6 output (LED)
static const PIN_ASSIGNMENT PIN_PA8 = { 8, GPIOA };                                                 // PA8 output (MCO)

//================================================================================================// divider
// GENERIC TERMINAL POSITIONS (LAB01 PART E/G)                                                      // UI constants
//================================================================================================// divider
#define PART_E_ROW     10                                                                           // row for Part E hex
#define PART_E_COL      5                                                                           // col for Part E hex
#define PART_G_WIDTH    6                                                                           // digits for Part G display

//================================================================================================// divider
// LAB01 PART F: SYSTICK 1ms TIMEBASE                                                               // SysTick helpers
//================================================================================================// divider
static volatile uint32_t g_ms = 0;                                                                  // global ms counter

void SysTick_Handler(void)                                                                          // SysTick interrupt handler
{                                                                                                   // start ISR
    g_ms++;                                                                                         // increment ms tick
}                                                                                                   // end ISR

static void Time_Init_1ms(void)                                                                     // init SysTick at 1ms
{                                                                                                   // start function
    SysTick->LOAD = (SystemCoreClock / 1000u) - 1u;                                                  // reload count for 1ms
    SysTick->VAL  = 0u;                                                                              // clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;// enable SysTick + IRQ
}                                                                                                   // end function

static uint32_t millis(void)                                                                        // return ms uptime
{                                                                                                   // start function
    return g_ms;                                                                                    // return tick count
}                                                                                                   // end function

//================================================================================================// divider
// GENERIC GPIO HELPERS (ACTIVE-LOW LED/SWITCH TEMPLATE)                                            // helper functions
//================================================================================================// divider
static uint8_t ReadInput(GPIO_TypeDef *port, int pin)                                               // read IDR bit
{                                                                                                   // start function
    return (uint8_t)((port->IDR >> pin) & 1u);                                                       // return input state
}                                                                                                   // end function

static uint8_t ReadOutput(GPIO_TypeDef *port, int pin)                                              // read ODR bit
{                                                                                                   // start function
    return (uint8_t)((port->ODR >> pin) & 1u);                                                       // return output latch
}                                                                                                   // end function

static uint8_t SwitchIsPressed(const PIN_ASSIGNMENT *sw)                                            // active-low switch pressed?
{                                                                                                   // start function
    return (ReadInput(sw->port, sw->pin) == 0u);                                                     // pressed when 0
}                                                                                                   // end function

static uint8_t LedIsOn(const PIN_ASSIGNMENT *led)                                                   // active-low LED on?
{                                                                                                   // start function
    return (ReadOutput(led->port, led->pin) == 0u);                                                  // on when 0
}                                                                                                   // end function

//================================================================================================// divider
// GENERIC ANSI UI HELPERS (LAB01 PART D/F TEMPLATE)                                                // UI functions
//================================================================================================// divider
static void UI_DrawStatic(void)                                                                     // draw static UI once
{                                                                                                   // start function
    _USART_ClearScreen(USART2);                                                                     // clear terminal
    _USART_TxStringXY(USART2, 1, 1, "LAB01 UI");                                                    // title text
    _USART_TxStringXY(USART2, 1, 3, "Switch States : [ ] [ ]");                                    // switch label
    _USART_TxStringXY(USART2, 1, 4, "LED States    : [ ] [ ]");                                    // LED label
    _USART_TxStringXY(USART2, 1, 6, "Keys: 1 toggles LED_A, 2 toggles LED_B");                      // hint line
}                                                                                                   // end function

static void UI_PutBoxMark(uint8_t row, uint8_t col, uint8_t on)                                     // draw X or space
{                                                                                                   // start function
    _USART_SetCursor(USART2, row, col);                                                             // move cursor
    _USART_TxByte(USART2, on ? 'X' : ' ');                                                          // print box mark
}                                                                                                   // end function

//================================================================================================// divider
// ICA07: MCO CONFIG HELPERS (PA8 AF0 + HANDOUT CFGR METHOD)                                        // ICA07 helpers
//================================================================================================// divider
static void MCO_PinInit_PA8(void)                                                                   // configure PA8 as MCO
{                                                                                                   // start function
    _GPIO_ClockEnable(GPIOA);                                                                       // ensure GPIOA clock enabled
    _GPIO_SetPinMode(GPIOA, 8, _GPIO_PinMode_AlternateFunction);                                    // set PA8 to alternate function
    _GPIO_SetPinAlternateFunction(GPIOA, 8, 0);                                                     // select AF0 for MCO
}                                                                                                   // end function

//================================================================================================// divider
// MAIN                                                                                            // entry point
//================================================================================================// divider
int main(void)                                                                                      // program start
{                                                                                                   // start main

    _GPIO_ClockEnable(GPIOA);                                                                       // enable GPIOA clock
    _GPIO_ClockEnable(GPIOB);                                                                       // enable GPIOB clock
    _GPIO_ClockEnable(GPIOC);                                                                       // enable GPIOC clock
    _USART_Init_USART2(SystemCoreClock, 38400);                                                     // initialize USART2 for terminal

//================================================================================================// divider
// LAB01 PART A (PRINT NAME + COURSE)                                                               // Part A
//================================================================================================// divider
#ifdef RUN_LAB01_PART_A                                                                             // compile only if enabled
    _USART_ClearScreen(USART2);                                                                     // clear terminal screen
    _USART_TxStringXY(USART2, 1, 1, "Your Name");                                                   // print your name at top-left
    _USART_TxStringXY(USART2, TERM_COLS - 7, TERM_ROWS, "CMPE1250");                                // print course at bottom-right
    while (1) { }                                                                                   // stop forever
#endif                                                                                              // end Part A

//================================================================================================// divider
// LAB01 PART B (UART ECHO + UPPER->LOWER)                                                          // Part B
//================================================================================================// divider
#ifdef RUN_LAB01_PART_B                                                                             // compile only if enabled
    char c;                                                                                         // received character
    const char CASE_MASK = ('a' ^ 'A');                                                             // XOR mask for case flip
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        if (_USART_RxByte(USART2, &c))                                                              // non-blocking receive
        {                                                                                           // start if
            if (c > ' ')                                                                            // ignore whitespace/control
            {                                                                                       // start if
                if (c >= 'A' && c <= 'Z')                                                           // if uppercase
                    c ^= CASE_MASK;                                                                 // convert to lowercase
                _USART_TxByte(USART2, c);                                                           // echo back
            }                                                                                       // end if
        }                                                                                           // end if
        if (USART2->ISR & USART_ISR_ORE)                                                            // if overrun error set
        {                                                                                           // start if
            USART2->ICR |= USART_ICR_ORECF;                                                         // clear overrun error
        }                                                                                           // end if
    }                                                                                               // end loop
#endif                                                                                              // end Part B

//================================================================================================// divider
// LAB01 PART C (DIGIT ADDS; OTHER TOGGLES A PIN)                                                   // Part C
//================================================================================================// divider
#ifdef RUN_LAB01_PART_C                                                                             // compile only if enabled
    _GPIO_SetPinMode(LED_A.port, LED_A.pin, _GPIO_PinMode_Output);                                  // set LED_A pin as output
    uint32_t counter = 0;                                                                           // counter storage
    char out[32];                                                                                   // output buffer
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        char key = _USART_RxByteB(USART2);                                                          // blocking key read
        if (key >= '0' && key <= '9')                                                               // if numeric
            counter += (uint32_t)(key - '0');                                                       // add numeric value
        else                                                                                        // otherwise
            _GPIO_PinToggle(LED_A.port, LED_A.pin);                                                 // toggle output pin
        sprintf(out, "%06lu\r\n", (unsigned long)counter);                                          // format 6-digit counter
        _USART_TxString(USART2, out);                                                               // print counter
    }                                                                                               // end loop
#endif                                                                                              // end Part C

//================================================================================================// divider
// LAB01 PART D (LIVE LED STATE DISPLAY)                                                            // Part D
//================================================================================================// divider
#ifdef RUN_LAB01_PART_D                                                                             // compile only if enabled
    _GPIO_SetPinMode(LED_A.port, LED_A.pin, _GPIO_PinMode_Output);                                  // set LED_A output
    _GPIO_SetPinMode(LED_B.port, LED_B.pin, _GPIO_PinMode_Output);                                  // set LED_B output
    UI_DrawStatic();                                                                                // draw static UI
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        UI_PutBoxMark(3, 18, LedIsOn(&LED_A));                                                       // update LED_A box
        UI_PutBoxMark(3, 22, LedIsOn(&LED_B));                                                       // update LED_B box
    }                                                                                               // end loop
#endif                                                                                              // end Part D

//================================================================================================// divider
// LAB01 PART E (HEX COUNTER AT FIXED POSITION)                                                     // Part E
//================================================================================================// divider
#ifdef RUN_LAB01_PART_E                                                                             // compile only if enabled
    uint16_t count = 0;                                                                             // 16-bit counter
    char out[16];                                                                                   // output buffer
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        _USART_SetCursor(USART2, PART_E_ROW, PART_E_COL);                                           // position cursor
        sprintf(out, "%04X", (unsigned int)count);                                                  // format hex
        _USART_TxString(USART2, out);                                                               // print hex
        count++;                                                                                    // increment
        Delay(750000);                                                                              // delay for visibility
    }                                                                                               // end loop
#endif                                                                                              // end Part E

//================================================================================================// divider
// LAB01 PART F (NON-BLOCKING RX + SWITCH UI + LOOP COUNTS)                                         // Part F
//================================================================================================// divider
#ifdef RUN_LAB01_PART_F                                                                             // compile only if enabled
    _GPIO_SetPinMode(LED_A.port, LED_A.pin, _GPIO_PinMode_Output);                                  // LED_A output
    _GPIO_SetPinMode(LED_B.port, LED_B.pin, _GPIO_PinMode_Output);                                  // LED_B output
    _GPIO_SetPinMode(SWITCH_A.port, SWITCH_A.pin, _GPIO_PinMode_Input);                             // SWITCH_A input
    _GPIO_SetPinMode(SWITCH_B.port, SWITCH_B.pin, _GPIO_PinMode_Input);                             // SWITCH_B input
    Time_Init_1ms();                                                                                // init SysTick 1ms
    UI_DrawStatic();                                                                                // draw static UI
    uint32_t loops = 0;                                                                             // loop counter
    uint32_t last = 0;                                                                              // last ms
    char line[64];                                                                                  // text buffer
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        loops++;                                                                                    // count loops
        char rx;                                                                                    // rx char
        if (_USART_RxByte(USART2, &rx))                                                             // non-blocking rx
        {                                                                                           // start if
            if (rx == '1') _GPIO_PinToggle(LED_A.port, LED_A.pin);                                  // toggle LED_A
            if (rx == '2') _GPIO_PinToggle(LED_B.port, LED_B.pin);                                  // toggle LED_B
        }                                                                                           // end if
        UI_PutBoxMark(3, 18, SwitchIsPressed(&SWITCH_A));                                           // update SW_A box
        UI_PutBoxMark(3, 22, SwitchIsPressed(&SWITCH_B));                                           // update SW_B box
        UI_PutBoxMark(4, 18, LedIsOn(&LED_A));                                                       // update LED_A box
        UI_PutBoxMark(4, 22, LedIsOn(&LED_B));                                                       // update LED_B box
        if ((millis() - last) >= 1000u)                                                             // every 1 sec
        {                                                                                           // start if
            last = millis();                                                                        // update timestamp
            snprintf(line, sizeof(line), "loops=%lu      ", (unsigned long)loops);                  // format loops
            _USART_TxStringXY(USART2, 1, 8, line);                                                  // print loops line
        }                                                                                           // end if
    }                                                                                               // end loop
#endif                                                                                              // end Part F

//================================================================================================// divider
// LAB01 PART G (CENTERED 6-DIGIT COUNTER + KEY ACTIONS)                                            // Part G
//================================================================================================// divider
#ifdef RUN_LAB01_PART_G                                                                             // compile only if enabled
    _GPIO_SetPinMode(PIN_PC6.port, PIN_PC6.pin, _GPIO_PinMode_Output);                              // PC6 output
    uint8_t midRow = (uint8_t)(TERM_ROWS / 2);                                                      // compute center row
    uint8_t midCol = (uint8_t)((TERM_COLS / 2) - (PART_G_WIDTH / 2));                               // compute center col
    uint32_t counter = 0;                                                                           // counter storage
    char out[16];                                                                                   // output buffer
    _USART_TxStringXY(USART2, 1, 1, "LAB01 G");                                                     // title line
    _USART_SetCursor(USART2, midRow, midCol);                                                       // go to center
    sprintf(out, "%06lu", (unsigned long)counter);                                                  // format
    _USART_TxString(USART2, out);                                                                   // print
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        char key = _USART_RxByteB(USART2);                                                          // blocking rx
        if (key >= '0' && key <= '9')                                                               // numeric?
            counter += (uint32_t)(key - '0');                                                       // add value
        else                                                                                        // non numeric
            _GPIO_PinToggle(PIN_PC6.port, PIN_PC6.pin);                                             // toggle LED
        _USART_SetCursor(USART2, midRow, midCol);                                                   // overwrite position
        sprintf(out, "%06lu", (unsigned long)counter);                                              // format again
        _USART_TxString(USART2, out);                                                               // print again
        Delay(750000);                                                                              // delay
    }                                                                                               // end loop
#endif                                                                                              // end Part G

//================================================================================================// divider
// ICA07 PART A (BASELINE SYSCLK + MCO SYSCLK/4 ON PA8)                                             // ICA07 A
//================================================================================================// divider
#ifdef RUN_ICA07_PART_A                                                                             // compile only if enabled

    _GPIO_SetPinMode(PIN_PA5.port, PIN_PA5.pin, _GPIO_PinMode_Output);                              // set PA5 output (scope)
    _GPIO_SetPinMode(PIN_PC6.port, PIN_PC6.pin, _GPIO_PinMode_Output);                              // set PC6 output (human LED)

    MCO_PinInit_PA8();                                                                              // configure PA8 as AF0 (MCO)

    //--------------------------------------------------------------------------------------------// divider
    // MCO CONFIG (HANDOUT VERBATIM)                                                                // required config
    //--------------------------------------------------------------------------------------------// divider
    unsigned int cfgrtemp = RCC->CFGR;                                                              // read CFGR
    cfgrtemp &= 0b00000000000000000000000000111111;                                                 // clear MCO bits (handout mask)
    cfgrtemp |= 0b01110001000000000000000000000000;                                                 // set MCO=SYSCLK, prescaler=/4 (HANDOUT)
    RCC->CFGR = cfgrtemp;                                                                           // write CFGR back

    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        _GPIO_PinToggle(PIN_PA5.port, PIN_PA5.pin);                                                 // toggle PA5 (scope)
        _GPIO_PinToggle(PIN_PC6.port, PIN_PC6.pin);                                                 // toggle PC6 (human)
        Delay(800000);                                                                              // tune to 200ms
    }                                                                                               // end loop

#endif                                                                                              // end ICA07 A

//================================================================================================// divider
// ICA07 PART B (PLL 64MHz + "CHANGE NOTHING ELSE" + VERIFY WITH MCO)                               // ICA07 B
//================================================================================================// divider
#ifdef RUN_ICA07_PART_B                                                                             // compile only if enabled

    //--------------------------------------------------------------------------------------------// divider
    // STEP 1: INCREASE SYSCLK FIRST                                                                // must be first
    //--------------------------------------------------------------------------------------------// divider
    Clock_InitPll(PLL_64MHZ);                                                                       // switch SYSCLK to 64MHz

    //--------------------------------------------------------------------------------------------// divider
    // STEP 2: (OPTIONAL) UART RE-INIT (ONLY FOR TERMINAL PRINTS)                                   // uart note
    //--------------------------------------------------------------------------------------------// divider
    // NOTE: If you don't need prints, you can remove this line to keep "changed nothing else".    // lab note
    _USART_Init_USART2(64000000u, 38400);                                                           // keep UART stable after PLL

    //--------------------------------------------------------------------------------------------// divider
    // STEP 3: GPIO OUTPUTS (SAME AS PART A)                                                        // gpio setup
    //--------------------------------------------------------------------------------------------// divider
    _GPIO_SetPinMode(PIN_PA5.port, PIN_PA5.pin, _GPIO_PinMode_Output);                              // PA5 output (scope)
    _GPIO_SetPinMode(PIN_PC6.port, PIN_PC6.pin, _GPIO_PinMode_Output);                              // PC6 output (human)

    //--------------------------------------------------------------------------------------------// divider
    // STEP 4: MCO OUTPUT ON PA8 (SAME HANDOUT CONFIG)                                              // mco setup
    //--------------------------------------------------------------------------------------------// divider
    MCO_PinInit_PA8();                                                                              // PA8 AF0 for MCO

    unsigned int cfgrtemp = RCC->CFGR;                                                              // read CFGR
    cfgrtemp &= 0b00000000000000000000000000111111;                                                 // clear MCO bits (handout mask)
    cfgrtemp |= 0b01110001000000000000000000000000;                                                 // MCO=SYSCLK, prescaler=/4 (HANDOUT)
    RCC->CFGR = cfgrtemp;                                                                           // write CFGR back

    //--------------------------------------------------------------------------------------------// divider
    // STEP 5: MAIN LOOP (SAME CODE + SAME DELAY VALUE AS PART A)                                   // measurement loop
    //--------------------------------------------------------------------------------------------// divider
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        _GPIO_PinToggle(PIN_PA5.port, PIN_PA5.pin);                                                 // toggle PA5 (scope speed)
        _GPIO_PinToggle(PIN_PC6.port, PIN_PC6.pin);                                                 // toggle PC6 (human)
        Delay(800000);                                                                              // SAME value as Part A
    }                                                                                               // end loop

#endif                                                                                              // end ICA07 B

//================================================================================================// divider
// ICA07 PART C (LSI/64 ON PA8)                                                                     // ICA07 C
//================================================================================================// divider
#ifdef RUN_ICA07_PART_C                                                                             // compile only if enabled
    MCO_PinInit_PA8();                                                                              // configure PA8 for MCO
    Clock_EnableOutput(MCO_Sel_LSI, MCO_Div64);                                                      // output LSI/64 on PA8
    while (1)                                                                                       // loop forever
    {                                                                                               // start loop
        ;                                                                                           // nothing to do
    }                                                                                               // end loop
#endif 
// ICA07 PART D (SYSCLK=64MHz + TIM14 CH1 PWM = 10kHz, 25% DUTY)                                  // ICA07 D
//================================================================================================// divider
#ifdef RUN_ICA07_PART_D                                                                            // compile only if enabled

    //--------------------------------------------------------------------------------------------// divider
    // STEP 1: SET SYSCLK = 64MHz (MUST BE FIRST)                                                  // clock setup
    //--------------------------------------------------------------------------------------------// divider
    Clock_InitPll(PLL_64MHZ);                                                                      // set SYSCLK to 64MHz
    SystemCoreClock = 64000000u;                                                                   // force variable if library doesn't update

    //--------------------------------------------------------------------------------------------// divider
    // STEP 2: ENABLE PERIPHERAL CLOCKS                                                            // RCC enable
    //--------------------------------------------------------------------------------------------// divider
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;                                                            // enable GPIOA clock
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;                                                           // enable TIM14 clock (APB2)

    //--------------------------------------------------------------------------------------------// divider
    // STEP 3: MAP TIM14_CH1 TO GPIO PIN (PA4 AF4)                                                 // pin mux
    //--------------------------------------------------------------------------------------------// divider
    _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_AlternateFunction);                                   // PA4 alternate function mode
    _GPIO_SetPinAlternateFunction(GPIOA, 4, 4);                                                    // PA4 AF4 = TIM14_CH1 (common mapping)

    //--------------------------------------------------------------------------------------------// divider
    // STEP 4: CONFIGURE TIM14 FOR PWM                                                             // timer config
    //--------------------------------------------------------------------------------------------// divider
    // Goal: PWM = 10kHz, Duty = 25%
    // SYSCLK = 64MHz -> choose TIM tick = 1MHz using PSC=63 (div by 64)
    // Tick = 64MHz / (63+1) = 1,000,000 Hz
    // Period counts = 1MHz / 10kHz = 100 counts -> ARR = 100-1 = 99
    // Duty 25% -> CCR1 = 25 counts (out of 100)

    TIM14->CR1   = 0u;                                                                             // stop timer, reset control
    TIM14->PSC   = 63u;                                                                            // prescaler to make 1MHz tick
    TIM14->ARR   = 99u;                                                                            // auto-reload for 10kHz period
    TIM14->CCR1  = 25u;                                                                            // compare for 25% duty

    TIM14->CCMR1 = 0u;                                                                             // clear CH1 mode config
    TIM14->CCMR1 |= (6u << 4);                                                                     // OC1M=110: PWM mode 1
    TIM14->CCMR1 |= (1u << 3);                                                                     // OC1PE=1: preload enable for CCR1

    TIM14->CCER  = 0u;                                                                             // clear enable/polarity
    TIM14->CCER |= (1u << 0);                                                                      // CC1E=1: enable CH1 output

    TIM14->EGR   = 1u;                                                                             // UG=1: load PSC/ARR/CCR into active regs
    TIM14->CR1  |= (1u << 7);                                                                      // ARPE=1: preload enable for ARR
    TIM14->CR1  |= (1u << 0);                                                                      // CEN=1: start timer

    //--------------------------------------------------------------------------------------------// divider
    // STEP 5: OPTIONAL TERMINAL PRINT                                                             // info line
    //--------------------------------------------------------------------------------------------// divider
    _USART_Init_USART2(SystemCoreClock, 38400u);                                                   // re-init UART for new SYSCLK
    _USART_TxString(USART2, "ICA07 D: TIM14_CH1 PWM on PA4 (AF4) = 10kHz, 25% duty\r\n");          // one-line status

    while (1)                                                                                      // loop forever
    {                                                                                              // start loop
        ;                                                                                          // PWM runs in hardware
    }                                                                                              // end loop

#endif  
//================================================================================================//
// ICA07 PART E                                                                                    //
// PWM DUTY CONTROL USING SWITCH TRANSITIONS (EDGE-BASED) + CLAMP                                  //
//================================================================================================//
#ifdef RUN_ICA07_PART_E                                                                            // compile only if enabled

    //--------------------------------------------------------------------------------------------//
    // STEP 1: SET SYSTEM CLOCK TO 64 MHz (REQUIRED FOR PART E)                                    //
    //--------------------------------------------------------------------------------------------//
    Clock_InitPll(PLL_64MHZ);                                                                      // switch SYSCLK to 64 MHz
    SystemCoreClock = 64000000u;                                                                   // force variable (safe for lab)

    //--------------------------------------------------------------------------------------------//
    // STEP 2: DEFINE SWITCH PINS (GENERIC TEMPLATE – EDIT TO MATCH YOUR BOARD)                    //
    //--------------------------------------------------------------------------------------------//
    // NOTE: Switches are ACTIVE-LOW (pressed = 0)
    const PIN_ASSIGNMENT S1 = { 10, GPIOA };                                                       // S1 pin (example)
    const PIN_ASSIGNMENT S2 = { 1,  GPIOB };                                                       // S2 pin (example)

    _GPIO_ClockEnable(S1.port);                                                                    // enable GPIO clock for S1
    _GPIO_ClockEnable(S2.port);                                                                    // enable GPIO clock for S2

    _GPIO_SetPinMode(S1.port, S1.pin, _GPIO_PinMode_Input);                                        // configure S1 as input
    _GPIO_SetPinMode(S2.port, S2.pin, _GPIO_PinMode_Input);                                        // configure S2 as input

    //--------------------------------------------------------------------------------------------//
    // STEP 3: CONFIGURE TIM14 FOR PWM OUTPUT (CHANNEL 1)                                          //
    //--------------------------------------------------------------------------------------------//
    // Timer math:
    // SYSCLK = 64 MHz
    // Prescaler = 63  →  timer clock = 1 MHz
    // ARR = 99         →  PWM frequency = 10 kHz

    TIM14->PSC = 63u;                                                                              // prescaler → 1 MHz timer clock
    TIM14->ARR = 99u;                                                                              // auto-reload → 10 kHz PWM

    TIM14->CCMR1 &= ~(0x7u << 4);                                                                  // clear OC1M bits
    TIM14->CCMR1 |=  (0x6u << 4);                                                                  // PWM Mode 1
    TIM14->CCMR1 |=  (1u << 3);                                                                    // preload enable (OC1PE)

    TIM14->CCER  |=  (1u << 0);                                                                    // enable CH1 output
    TIM14->CR1   |=  (1u << 7);                                                                    // auto-reload preload enable
    TIM14->EGR   |=  (1u << 0);                                                                    // force register update

    //--------------------------------------------------------------------------------------------//
    // STEP 4: INITIAL DUTY CYCLE SETUP                                                            //
    //--------------------------------------------------------------------------------------------//
    uint32_t duty_pct = 25u;                                                                       // start duty = 25%

    // Convert duty % → CCR value
    TIM14->CCR1 = (duty_pct * (TIM14->ARR + 1u)) / 100u;                                           // set PWM duty

    TIM14->CR1 |= (1u << 0);                                                                       // enable timer (CEN)

    //--------------------------------------------------------------------------------------------//
    // STEP 5: TRANSITION (EDGE) DETECTION VARIABLES                                                //
    //--------------------------------------------------------------------------------------------//
    uint8_t s1_prev = ReadInput(S1.port, S1.pin);                                                  // previous S1 state
    uint8_t s2_prev = ReadInput(S2.port, S2.pin);                                                  // previous S2 state

    //--------------------------------------------------------------------------------------------//
    // STEP 6: MAIN LOOP – EDGE-BASED DUTY CONTROL                                                  //
    //--------------------------------------------------------------------------------------------//
    while (1)                                                                                      // loop forever
    {
        uint8_t s1_now = ReadInput(S1.port, S1.pin);                                               // read S1 current
        uint8_t s2_now = ReadInput(S2.port, S2.pin);                                               // read S2 current

        //----------------------------------------------------------------------------------------//
        // S1 TRANSITION: DECREASE DUTY BY 2%                                                     //
        //----------------------------------------------------------------------------------------//
        if ((s1_prev == 1u) && (s1_now == 0u))                                                     // detect falling edge
        {
            duty_pct = (duty_pct > 12u) ? (duty_pct - 2u) : 10u;                                   // decrement + clamp
            Delay(200000u);                                                                        // debounce delay
        }

        //----------------------------------------------------------------------------------------//
        // S2 TRANSITION: INCREASE DUTY BY 7%                                                     //
        //----------------------------------------------------------------------------------------//
        if ((s2_prev == 1u) && (s2_now == 0u))                                                     // detect falling edge
        {
            duty_pct = (duty_pct < 83u) ? (duty_pct + 7u) : 90u;                                   // increment + clamp
            Delay(200000u);                                                                        // debounce delay
        }

        //----------------------------------------------------------------------------------------//
        // SAFETY CLAMP (REQUIRED BY SPEC)                                                         //
        //----------------------------------------------------------------------------------------//
        if (duty_pct < 10u) duty_pct = 10u;                                                        // minimum duty
        if (duty_pct > 90u) duty_pct = 90u;                                                        // maximum duty

        //----------------------------------------------------------------------------------------//
        // APPLY NEW DUTY CYCLE                                                                    //
        //----------------------------------------------------------------------------------------//
        TIM14->CCR1 = (duty_pct * (TIM14->ARR + 1u)) / 100u;                                       // update PWM output

        //----------------------------------------------------------------------------------------//
        // SAVE STATES FOR NEXT TRANSITION CHECK                                                   //
        //----------------------------------------------------------------------------------------//
        s1_prev = s1_now;                                                                          // update S1 history
        s2_prev = s2_now;                                                                          // update S2 history
    }

#endif                                                                                             // end ICA07 PART E
                                                                                                                                      // end ICA07 C

    while (1) { }                                                                                   // safety infinite loop
}                                                                                                   // end main
