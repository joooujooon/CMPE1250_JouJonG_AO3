/*********************************************************************                         // File header block (course template)
*                    SEGGER Microcontroller GmbH                      // Vendor banner
*                        The Embedded Experts                         // Vendor banner
********************************************************************** // Separator
File    : main.c - LAB01 MASTER (Parts A-F)                            // File purpose/name
Author  : Jou Jon Galenzoga                                            // Author name
*********************************************************************/ // End header

#include <stdio.h>                                                     // printf/sprintf/snprintf support
#include <stdint.h>                                                    // fixed-width integer types (uint32_t etc.)
#include "stm32g031xx.h"                                               // MCU register definitions (CMSIS)
#include "gpio.h"                                                      // your GPIO helper library
#include "usart.h"                                                     // your USART helper library

// ================================================================     // Section divider
// SELECT WHICH PART TO RUN (ENABLE ONLY ONE)                           // Choose which lab part compiles
// ================================================================     // Section divider
//#define RUN_PART_A                                                   // Enable Part A (terminal text)
 //#define RUN_PART_B                                                  // Enable Part B (lowercase echo)
 //#define RUN_PART_C                                                  // Enable Part C (blocking key + counter)
 //#define RUN_PART_D                                                  // Enable Part D (LED status UI)
 //#define RUN_PART_E                                                  // Enable Part E (hex counter + wave)
//#define RUN_PART_F                                                     // Enable Part F (enhancements)
#define RUN_PART_G

// ================================================================     // Section divider
// LAB01 REQUIREMENT: PIN ASSIGNMENT STRUCT                             // Required struct for flexible pin mapping
// ================================================================     // Section divider
typedef struct                                                         // Define a struct type
{                                                                       // Start struct body
    int pin_number;                                                    // pin number (0-15)
    GPIO_TypeDef *pin_port;                                            // pointer to GPIO port (GPIOA/GPIOB/...)
} PIN_ASSIGNMENT;                                                      // struct type name

// ================================================================     // Section divider
// YOUR WIRING (HC-35)                                                  // Actual wiring mapping for your I/O board
// ================================================================     // Section divider
const PIN_ASSIGNMENT LED_A    = { 0, GPIOB };                           // const PIN_ASSIGNMENT LED_A    = { pin#, GPIOx };  (D1 → PB0)
const PIN_ASSIGNMENT LED_B    = { 2, GPIOB };                           // const PIN_ASSIGNMENT LED_B    = { pin#, GPIOx };  (D2 → PB2)
const PIN_ASSIGNMENT SWITCH_A = { 10, GPIOA };                          // const PIN_ASSIGNMENT SWITCH_A = { pin#, GPIOx };  (S1 → PA10)
const PIN_ASSIGNMENT SWITCH_B = { 1, GPIOB };                           // const PIN_ASSIGNMENT SWITCH_B = { pin#, GPIOx };  (S2 → PB1)

// ================================================================     // Section divider
// PART E SETTINGS (HEX COUNTER + WAVEFORM PIN)                          // Settings for Part E output position + scope pin
// ================================================================     // Section divider
#define PART_E_ROW   10                                                 // terminal row where hex count prints
#define PART_E_COL   5                                                  // terminal column where hex count prints
#define WAVE_PORT    GPIOA                                              // waveform GPIO port (toggle pin) PA bacause its A
#define WAVE_PIN     8                                                  // waveform GPIO pin number (PA8 / Arduino D9)

// ================================================================     // Section divider
// PART F UI SETTINGS (MOVE UI BY CHANGING ONLY THESE TWO)               // UI layout can be moved using these base coords
// ================================================================     // Section divider
#define UI_BASE_ROW   2                                                 // top row anchor for the UI block
#define UI_BASE_COL   2                                                 // left column anchor for the UI block

#define UI_ROW_SWITCH     (UI_BASE_ROW + 0)                             // row for "Switch States"
#define UI_ROW_LED        (UI_BASE_ROW + 1)                             // row for "LED Control"
#define UI_ROW_HINT       (UI_BASE_ROW + 3)                             // row for user hint text
#define UI_ROW_LOOPS      (UI_BASE_ROW + 6)                             // row for loop counter display
#define UI_ROW_CHANGES    (UI_BASE_ROW + 7)                             // row for state change counter display

#define OFF_BOX1          16                                            // column offset for first [ ] box
#define OFF_BOX2          20                                            // column offset for second [ ] box
#define UI_COL_SW1        (UI_BASE_COL + OFF_BOX1)                      // column for switch 1 box character
#define UI_COL_SW2        (UI_BASE_COL + OFF_BOX2)                      // column for switch 2 box character
#define UI_COL_LED1       (UI_BASE_COL + OFF_BOX1)                      // column for LED 1 box character
#define UI_COL_LED2       (UI_BASE_COL + OFF_BOX2)                      // column for LED 2 box character

// Part G
#define PART_G_WIDTH   6                      // total digits shown
#define PART_G_DELAY   750000                 // Delay() tuning value (adjust as needed)

// LED pin to toggle for "non-numeric" keys
// If your board truly has PC6 available, keep this.
// If your NUCLEO-G031K8 does NOT have PC6 accessible, change to a real pin (ex: PA8).

#define PART_G_LED_PORT   GPIOC               // GPIOx
#define PART_G_LED_PIN  6                     // pin# i can edit pins here

// ================================================================     // Section divider
// COMMON HELPERS (USED BY MULTIPLE PARTS)                               // Shared helper functions for several parts
// ================================================================     // Section divider
static void Delay(volatile uint32_t d)                                  // busy-wait delay (rough timing)
{                                                                       // start delay function
    while (d--);                                                        // decrement until zero
}                                                                       // end delay function

// ---------- SysTick 1ms timebase (needed for PART F counters) ---------- // comment: SysTick used as millisecond timer
static volatile uint32_t g_ms = 0;                                      // global millisecond counter

void SysTick_Handler(void)                                              // SysTick ISR (runs every 1ms if configured)
{                                                                       // start ISR
    g_ms++;                                                             // increment ms counter
}                                                                       // end ISR

static void Time_Init_1ms(void)                                         // configure SysTick for 1ms tick
{                                                                       // start init
    SysTick->LOAD  = (SystemCoreClock / 1000u) - 1u;                    // reload value for 1ms period
    SysTick->VAL   = 0u;                                                // clear current value
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |                       // use CPU clock
                     SysTick_CTRL_TICKINT_Msk   |                       // enable interrupt
                     SysTick_CTRL_ENABLE_Msk;                           // enable SysTick
}                                                                       // end init

static uint32_t millis(void)                                            // returns ms since start
{                                                                       // start millis
    return g_ms;                                                        // return global ms counter
}                                                                       // end millis

// ---------- Pin read helpers (active-low switches + active-low LEDs) ---------- // comment: helpers to read GPIO pins
static inline uint8_t ReadPin(GPIO_TypeDef *port, int pin)              // reads input state from IDR
{                                                                       // start ReadPin
    return (uint8_t)((port->IDR >> pin) & 1u);                          // shift+mask chosen bit
}                                                                       // end ReadPin

static inline uint8_t ReadOut(GPIO_TypeDef *port, int pin)              // reads output latch state from ODR
{                                                                       // start ReadOut
    return (uint8_t)((port->ODR >> pin) & 1u);                          // shift+mask chosen bit
}                                                                       // end ReadOut

static inline uint8_t SwitchPressed(const PIN_ASSIGNMENT *sw)           // returns 1 if switch pressed
{                                                                       // start SwitchPressed
    return (ReadPin(sw->pin_port, sw->pin_number) == 0u);               // active-low: pressed = 0
}                                                                       // end SwitchPressed

static inline uint8_t LedIsOn(const PIN_ASSIGNMENT *led)                // returns 1 if LED is ON
{                                                                       // start LedIsOn
    return (ReadOut(led->pin_port, led->pin_number) == 0u);             // active-low: ON = 0
}                                                                       // end LedIsOn

// ---------- UI helpers (PART D / F) ----------                         // comment: terminal drawing helpers
static void RenderStaticUI(void)                                        // prints static UI lines once
{                                                                       // start RenderStaticUI
    _USART_ClearScreen(USART2);                                         // clear the terminal screen
    _USART_TxStringXY(USART2, UI_BASE_COL, UI_ROW_SWITCH, "Switch States : [ ] [ ]"); // print switch row
    _USART_TxStringXY(USART2, UI_BASE_COL, UI_ROW_LED,    "LED Control   : [ ] [ ]"); // print led row
    _USART_TxStringXY(USART2, UI_BASE_COL, UI_ROW_HINT,   "Press 1 or 2 to toggle LEDs"); // print hint row
}                                                                       // end RenderStaticUI

static void RenderOneChar(uint8_t row, uint8_t col, char ch)            // prints one character at a location
{                                                                       // start RenderOneChar
    _USART_SetCursor(USART2, row, col);                                 // move cursor to row/col
    _USART_TxByte(USART2, ch);                                          // transmit single character
}                                                                       // end RenderOneChar

static void FormatEngineering(uint64_t value, char *out, size_t outSize)// formats loop count in engineering notation
{                                                                       // start FormatEngineering
    if (value == 0)                                                     // if loop count is zero
    {                                                                   // start if
        snprintf(out, outSize, "0 loops of main");                      // print simple message
        return;                                                         // exit function early
    }                                                                   // end if

    int exp = 0;                                                        // exponent in multiples of 3
    double mant = (double)value;                                        // mantissa as floating point
    while (mant >= 1000.0)                                              // while mantissa too large
    {                                                                   // start while
        mant /= 1000.0;                                                 // scale down by 1000
        exp += 3;                                                       // increase exponent by 3
    }                                                                   // end while

    snprintf(out, outSize, "%.3fx10^%d loops of main", mant, exp);      // build engineering string
}                                                                       // end FormatEngineering

// ================================================================     // Section divider
// MAIN                                                                  // Program entry point
// ================================================================     // Section divider
int main(void)                                                          // main function
{                                                                       // start main
    // ------------------------------------------------------------    // divider: common init
    // COMMON INIT (used by most parts)                                // shared init work
    // ------------------------------------------------------------    // divider
    _GPIO_ClockEnable(GPIOA);                                           // enable clock for GPIOA
    _GPIO_ClockEnable(GPIOB);                                           // enable clock for GPIOB

    _USART_Init_USART2(SystemCoreClock, 38400);                         // init USART2 at 38400 baud

    // ============================================================    // divider: Part A
    // PART A                                                           // part A description
    // Clear screen, print name top-left, course bottom-right           // what Part A does
    // ============================================================    // divider
#ifdef RUN_PART_A                                                       // compile Part A only if enabled

    _USART_ClearScreen(USART2);                                         // clear terminal
    _USART_TxStringXY(USART2, 1, 1, "Jou Jon Galenzoga");               // print name at (1,1)

    const char *courseText = "CMPE1250";                                // course label string
    int col = _USART_COLS - 7;                                          // compute rightmost column for 7 chars
    int row = _USART_ROWS;                                              // bottom row of terminal
    _USART_TxStringXY(USART2, col, row, courseText);                    // print course bottom-right

    while (1);                                                          // stay here forever

#endif                                                                  // end Part A compile block
    // ========================== END PART A =======================   // divider label


    // ============================================================    // divider: Part B
    // PART B                                                           // part B description
    // Lowercase echo (non-blocking RX) + overrun handling              // what Part B does
    // ============================================================    // divider
#ifdef RUN_PART_B                                                       // compile Part B only if enabled

    char c;                                                             // received character buffer
    const char CASE_MASK = ('a' ^ 'A');                                 // XOR mask for case conversion

    while (1)                                                           // loop forever
    {
        if (_USART_RxByte(USART2, &c))                                  // if a byte was received (non-blocking)
        {
            if (c > ' ')                                                // ignore spaces/control chars
            {
                if (c >= 'A' && c <= 'Z')                               // if uppercase letter
                    c ^= CASE_MASK;                                     // convert to lowercase via XOR

                _USART_TxByte(USART2, c);                               // echo character back
            }
        }

        if (USART2->ISR & USART_ISR_ORE)                                // if overrun error occurred
        {
            if (!(USART2->ISR & USART_ISR_RXNE_RXFNE))                  // if RX register not full
                USART2->ICR |= USART_ICR_ORECF;                         // clear overrun error flag
        }
    }

#endif                                                                  // end Part B compile block
    // ========================== END PART B =======================   // divider label


    // ============================================================    // divider: Part C
    // PART C                                                           // part C description
    // Blocking key read: digits add to counter, others toggle pin,      // what Part C does
    // prints 6-digit decimal counter each key press                    // what Part C prints
    // ============================================================    // divider
#ifdef RUN_PART_C                                                       // compile Part C only if enabled

    uint32_t counter = 0;                                               // numeric counter total
    char key;                                                           // received key
    char out[16];                                                       // output string buffer

    _GPIO_SetPinMode(WAVE_PORT, WAVE_PIN, _GPIO_PinMode_Output);        // configure wave pin as output

    while (1)                                                           // loop forever
    {
        key = _USART_RxByteB(USART2);                                   // blocking receive (waits for key)

        if (key >= '0' && key <= '9')                                   // if key is a digit
            counter += (key - '0');                                     // add digit value to counter
        else                                                            // otherwise (non-digit)
        {
            _GPIO_PinToggle(WAVE_PORT, WAVE_PIN);                       // toggle output pin
            Delay(600000);                                              // short delay for visible toggle
        }

        sprintf(out, "%06lu\r\n", (unsigned long)counter);              // format 6-digit decimal with CRLF
        _USART_TxString(USART2, out);                                   // send formatted string
    }

#endif                                                                  // end Part C compile block
    // ========================== END PART C =======================   // divider label


    // ============================================================    // divider: Part D
    // PART D                                                           // part D description
    // Render LED states only (simple UI) – continuous update           // what Part D does
    // ============================================================    // divider
#ifdef RUN_PART_D                                                       // compile Part D only if enabled

    _GPIO_SetPinMode(LED_A.pin_port, LED_A.pin_number, _GPIO_PinMode_Output); // set LED_A pin as output
    _GPIO_SetPinMode(LED_B.pin_port, LED_B.pin_number, _GPIO_PinMode_Output); // set LED_B pin as output

    RenderStaticUI();                                                   // draw the fixed UI text once

    while (1)                                                           // loop forever
    {
        uint8_t led1 = LedIsOn(&LED_A);                                 // read LED_A state (active-low)
        uint8_t led2 = LedIsOn(&LED_B);                                 // read LED_B state (active-low)

        RenderOneChar(UI_ROW_LED, UI_COL_LED1, led1 ? 'X' : ' ');       // draw LED1 box char
        RenderOneChar(UI_ROW_LED, UI_COL_LED2, led2 ? 'X' : ' ');       // draw LED2 box char
    }

#endif                                                                  // end Part D compile block
    // ========================== END PART D =======================   // divider label


    // ============================================================    // divider: Part E
    // PART E                                                           // part E description
    // HEX counter at row/col, increments every ~250ms,                 // what Part E does
    // toggles waveform pin each update, SPACE resets                   // additional behavior
    // ============================================================    // divider
#ifdef RUN_PART_E                                                       // compile Part E only if enabled

    _GPIO_SetPinMode(WAVE_PORT, WAVE_PIN, _GPIO_PinMode_Output);        // configure wave pin as output

    uint16_t count = 0;                                                 // 16-bit counter
    char out[16];                                                       // output string buffer
    char rx;                                                            // received character

    _USART_TxStringXY(USART2, 1, 1, "PART E: HEX @ row10 col5 (SPACE resets)"); // print label once

    while (1)                                                           // loop forever
    {
        if (_USART_RxByte(USART2, &rx))                                 // if a key was received (non-blocking)
        {
            if (rx == ' ')                                              // if SPACE pressed
                count = 0;                                              // reset counter
        }

        _USART_SetCursor(USART2, PART_E_ROW, PART_E_COL);               // move cursor to row/col
        sprintf(out, "%04X", (unsigned int)count);                      // format as 4-digit hex with zeros
        _USART_TxString(USART2, out);                                   // print the hex number

        _GPIO_PinToggle(WAVE_PORT, WAVE_PIN);                           // toggle pin for scope waveform

        count++;                                                        // increment counter

        Delay(750000);                                                  // delay (tune with AD2 for ~250ms)
    }

#endif                                                                  // end Part E compile block
    // ========================== END PART E =======================   // divider label


    // ============================================================    // divider: Part F
    // PART F (ENHANCEMENTS)                                            // part F description
    // - No redundant redraw (only update on change)                    // enhancement: reduce redraw
    // - state changes counter                                          // enhancement: count changes
    // - loops counter in engineering notation, update <= 1 Hz          // enhancement: 1Hz loop print
    // - UI movable by UI_BASE_ROW / UI_BASE_COL only                   // enhancement: move UI
    // ============================================================    // divider
#ifdef RUN_PART_F                                                       // compile Part F only if enabled

    _GPIO_SetPinMode(LED_A.pin_port, LED_A.pin_number, _GPIO_PinMode_Output); // configure LED_A output
    _GPIO_SetPinMode(LED_B.pin_port, LED_B.pin_number, _GPIO_PinMode_Output); // configure LED_B output
    _GPIO_SetPinMode(SWITCH_A.pin_port, SWITCH_A.pin_number, _GPIO_PinMode_Input); // configure SWITCH_A input
    _GPIO_SetPinMode(SWITCH_B.pin_port, SWITCH_B.pin_number, _GPIO_PinMode_Input); // configure SWITCH_B input

    Time_Init_1ms();                                                    // start 1ms SysTick timing
    RenderStaticUI();                                                   // draw the fixed UI text once

    uint8_t prevSW1  = 2u, prevSW2  = 2u;                               // previous switch states (force initial draw)
    uint8_t prevLED1 = 2u, prevLED2 = 2u;                               // previous LED states (force initial draw)

    uint32_t stateChanges = 0;                                          // counts total state changes
    uint64_t loopCount = 0;                                             // counts total loop iterations

    uint32_t lastCounterUpdateMs = 0;                                   // last time counters were printed
    char line[64];                                                      // output line buffer

    while (1)                                                           // infinite loop
    {
        loopCount++;                                                    // increment loop counter each iteration

        // Keyboard toggles LEDs                                        // comment: UART input toggles outputs
        char rx;                                                        // received key
        if (_USART_RxByte(USART2, &rx))                                 // if key received (non-blocking)
        {
            if (rx == '1')                                              // if user typed '1'
                _GPIO_PinToggle(LED_A.pin_port, LED_A.pin_number);      // toggle LED_A
            else if (rx == '2')                                         // else if user typed '2'
                _GPIO_PinToggle(LED_B.pin_port, LED_B.pin_number);      // toggle LED_B
        }

        // Read current states                                          // comment: sample current hardware states
        uint8_t sw1  = SwitchPressed(&SWITCH_A);                        // read switch A pressed?
        uint8_t sw2  = SwitchPressed(&SWITCH_B);                        // read switch B pressed?
        uint8_t led1 = LedIsOn(&LED_A);                                 // read LED_A on?
        uint8_t led2 = LedIsOn(&LED_B);                                 // read LED_B on?

        // Update ONLY if changed                                       // comment: avoid redundant terminal writes
        if (sw1 != prevSW1)                                             // if switch A changed since last loop
        {
            RenderOneChar(UI_ROW_SWITCH, UI_COL_SW1, sw1 ? 'X' : ' ');  // draw X or space in switch1 box
            prevSW1 = sw1;                                              // store new state
            stateChanges++;                                             // count a state change
        }

        if (sw2 != prevSW2)                                             // if switch B changed since last loop
        {
            RenderOneChar(UI_ROW_SWITCH, UI_COL_SW2, sw2 ? 'X' : ' ');  // draw X or space in switch2 box
            prevSW2 = sw2;                                              // store new state
            stateChanges++;                                             // count a state change
        }

        if (led1 != prevLED1)                                           // if LED_A state changed since last loop
        {
            RenderOneChar(UI_ROW_LED, UI_COL_LED1, led1 ? 'X' : ' ');   // draw X or space in led1 box
            prevLED1 = led1;                                            // store new state
            stateChanges++;                                             // count a state change
        }

        if (led2 != prevLED2)                                           // if LED_B state changed since last loop
        {
            RenderOneChar(UI_ROW_LED, UI_COL_LED2, led2 ? 'X' : ' ');   // draw X or space in led2 box
            prevLED2 = led2;                                            // store new state
            stateChanges++;                                             // count a state change
        }

        // Update counters <= 1 Hz                                      // comment: update once per second only
        uint32_t now = millis();                                        // get current ms time
        if ((now - lastCounterUpdateMs) >= 1000u)                       // if 1000ms passed
        {
            lastCounterUpdateMs = now;                                  // update last time stamp

            FormatEngineering(loopCount, line, sizeof(line));           // build "x10^y loops of main" string
            _USART_TxStringXY(USART2, UI_BASE_COL, UI_ROW_LOOPS, line); // print loop count line

            snprintf(line, sizeof(line), "%lu state changes detected   ", // build state change string
                     (unsigned long)stateChanges);                      // convert to unsigned long for printf
            _USART_TxStringXY(USART2, UI_BASE_COL, UI_ROW_CHANGES, line); // print state change line
        }
    }

#endif                                                                  // end Part F compile block
    // ========================== END PART F =======================   // divider label
    // ============================================================
// PART G
// - Display a 6-digit counter in the middle (leading zeros)
// - No screen clear; overwrite only
// - Numeric key adds that digit to counter
// - Non-numeric key toggles LED on PC6
// ============================================================
#ifdef RUN_PART_G

    // Enable clocks for ports used
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOB);
    _GPIO_ClockEnable(GPIOC);

    // USART for terminal
    _USART_Init_USART2(SystemCoreClock, 38400);

    // LED output pin (PC6 as requested)
    _GPIO_SetPinMode(PART_G_LED_PORT, PART_G_LED_PIN, _GPIO_PinMode_Output);

    // Compute "middle" position using your terminal size macros
    // Row middle:
    uint8_t midRow = (uint8_t)(_USART_ROWS / 2);

    // Col middle, shift left by half the width so 6 chars are centered
    uint8_t midCol = (uint8_t)((_USART_COLS / 2) - (PART_G_WIDTH / 2));

    uint32_t counter = 0;                     // running total
    char key = 0;                             // last key received
    char out[16];                             // output buffer

    // Optional: Print a label once (no clearing later)
    _USART_TxStringXY(USART2, 1, 1, "PART G: 6-digit counter (0-9 adds, other toggles LED)");

    // Draw initial counter once
    _USART_SetCursor(USART2, midRow, midCol);
    sprintf(out, "%06lu", (unsigned long)counter);
    _USART_TxString(USART2, out);

    while (1)
    {
        // BLOCKING receive: waits for a key press
        key = _USART_RxByteB(USART2);

        // Numeric key: add digit value
        if (key >= '0' && key <= '9')
        {
            counter += (uint32_t)(key - '0');
        }
        // Any other key: toggle LED
        else
        {
            _GPIO_PinToggle(PART_G_LED_PORT, PART_G_LED_PIN);
        }

        // Overwrite counter in SAME spot (no screen clear)
        _USART_SetCursor(USART2, midRow, midCol);
        sprintf(out, "%06lu", (unsigned long)counter);
        _USART_TxString(USART2, out);

        // Delay between updates (tune if needed)
        Delay(PART_G_DELAY);
    }

#endif
// ========================== END PART G =======================

    while (1);                                                          // safety infinite loop (should never reach)
}                                                                       // end main

