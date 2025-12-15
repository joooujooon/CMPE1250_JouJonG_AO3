/*******************************************************************************
 * LAB 2: PWM FUNCTION GENERATOR - FINAL INTEGRATED VERSION
 * CMPE1250: Embedded Systems
 * 
 * AUTHOR: [Your Name]
 * DATE:   [Date]
 * 
 * DESCRIPTION:
 *   Complete PWM-based function generator with terminal interface.
 *   Features selectable frequencies, adjustable duty cycle, real-time clock,
 *   and optional button controls.
 * 
 * HARDWARE CONFIGURATION:
 *   PA4  (CN4-10) - PWM output (TIM14_CH1, AF4)
 *   PC6  (CN3-4)  - Status LED (indicates PWM active)
 *   PA2  (CN4-10) - USART2 TX → connects to USB-Serial RX
 *   PA3  (CN4-9)  - USART2 RX → connects from USB-Serial TX
 * 
 * OPTIONAL ENHANCEMENT (Uncomment ENABLE_BUTTONS):
 *   PA0  (CN4-12) - Button S1: Duty -1%
 *   PA1  (CN4-11) - Button S2: Duty +1%
 *   PA11 (CN4-7)  - Button S3: Previous frequency
 *   PA12 (CN4-8)  - Button S4: Next frequency
 * 
 * TERMINAL SETTINGS:
 *   Baud Rate: 115200 (or 192300 if your adapter supports it)
 *   Data: 8 bits, No parity, 1 stop bit
 *   Flow control: None
 * 
 * KEYBOARD CONTROLS:
 *   SPACE ......... Toggle PWM On/Off
 *   +/= ........... Increase duty by 5%
 *   - ............. Decrease duty by 5%
 *   0-9 ........... Set duty to 0%, 10%, 20%, ... 90%
 *   A ............. 100 Hz
 *   B ............. 500 Hz
 *   C ............. 1 kHz
 *   D ............. 5 kHz
 *   E ............. 10 kHz
 * 
 ******************************************************************************/

#include "stm32g031xx.h"
#include "clock.h"
#include "gpio.h"
#include "Timer.h"
#include "usart.h"
#include <stdio.h>

/*=============================================================================
 * CONFIGURATION SECTION
 * Uncomment/modify these defines to customize behavior
 *===========================================================================*/

// ┌─────────────────────────────────────────────────────────────────────────┐
// │ SYSTEM CLOCK CONFIGURATION                                              │
// │ Set to PLL_32MHZ or PLL_64MHZ (must match SYSCLK_FREQ below)           │
// └─────────────────────────────────────────────────────────────────────────┘
#define CLOCK_CONFIG        PLL_32MHZ
#define SYSCLK_FREQ         32000000U    // Must match CLOCK_CONFIG!

// ┌─────────────────────────────────────────────────────────────────────────┐
// │ BAUD RATE CONFIGURATION                                                 │
// │ Common values: 115200 (most compatible), 192300 (faster if supported)  │
// └─────────────────────────────────────────────────────────────────────────┘
#define BAUD_RATE           115200       // Change to 192300 if your adapter supports it

// ┌─────────────────────────────────────────────────────────────────────────┐
// │ FEATURE ENABLES                                                         │
// │ Uncomment to enable optional features                                  │
// └─────────────────────────────────────────────────────────────────────────┘
//#define ENABLE_BUTTONS              // Uncomment to enable 4-button enhancement
//#define ENABLE_FANCY_UI             // Uncomment for box-drawing characters (may not work on all terminals)

// ┌─────────────────────────────────────────────────────────────────────────┐
// │ PIN DEFINITIONS                                                         │
// │ Change these if you use different pins                                 │
// └─────────────────────────────────────────────────────────────────────────┘
#define PWM_PIN             4            // PA4 - TIM14_CH1 output
#define LED_PIN             6            // PC6 - Status LED

#ifdef ENABLE_BUTTONS
#define BTN_S1_PIN          0            // PA0 - Duty -1%
#define BTN_S2_PIN          1            // PA1 - Duty +1%
#define BTN_S3_PIN          11           // PA11 - Previous frequency
#define BTN_S4_PIN          12           // PA12 - Next frequency
#endif

/*=============================================================================
 * FREQUENCY TABLE
 * Defines the 5 selectable frequencies for PWM generation
 * 
 * Formula: PWM_freq = SYSCLK / (PSC × ARR)
 * 
 * Each entry contains:
 *   - key: Keyboard letter to select this frequency
 *   - freq_hz: Actual frequency in Hz
 *   - prescaler: Timer prescaler value (PSC register gets prescaler-1)
 *   - period: Timer period value (ARR register gets period-1)
 *===========================================================================*/

typedef struct {
    char key;              // Keyboard key (A-E)
    uint16_t freq_hz;      // Frequency in Hz
    uint16_t prescaler;    // Prescaler value (actual PSC = prescaler - 1)
    uint16_t period;       // Period value (actual ARR = period - 1)
} FrequencyConfig;

const FrequencyConfig FREQ_TABLE[] = {
    // Key, Freq,  PSC,  ARR     Formula verification:
    {'A', 100,    320,  1000},  // 32MHz / (320 × 1000) = 100 Hz
    {'B', 500,    64,   1000},  // 32MHz / (64 × 1000)  = 500 Hz
    {'C', 1000,   32,   1000},  // 32MHz / (32 × 1000)  = 1 kHz
    {'D', 5000,   64,   100},   // 32MHz / (64 × 100)   = 5 kHz
    {'E', 10000,  32,   100}    // 32MHz / (32 × 100)   = 10 kHz
};

#define NUM_FREQUENCIES     5

/*=============================================================================
 * APPLICATION STATE
 * Global structure holding current system state
 *===========================================================================*/

typedef struct {
    uint8_t freq_index;         // Current frequency index (0-4)
    uint8_t duty_percent;       // Current duty cycle (0-100%)
    uint8_t pwm_enabled;        // PWM state: 0=off, 1=on
    uint32_t uptime_seconds;    // Seconds since startup
} AppState;

// Initialize with default values
AppState g_state = {
    .freq_index = 0,            // Start at 100 Hz (index 0 = 'A')
    .duty_percent = 50,         // Start at 50% duty cycle
    .pwm_enabled = 0,           // Start with PWM disabled
    .uptime_seconds = 0         // Clock starts at 00:00:00
};

/*=============================================================================
 * FUNCTION PROTOTYPES
 *===========================================================================*/

// System initialization
void System_Init(void);

// PWM control functions
void PWM_Configure(uint8_t freq_index);
void PWM_UpdateDuty(uint8_t percent);
void PWM_Toggle(void);

// User interface functions
void UI_DrawHeader(void);
void UI_DrawStatus(void);
void UI_DrawControls(void);
void UI_Refresh(void);

// Utility functions
void Uptime_Update(void);
void Process_KeyPress(char key);

#ifdef ENABLE_BUTTONS
void Process_Buttons(void);
#endif

/*=============================================================================
 * MAIN FUNCTION
 *===========================================================================*/

int main(void)
{
    char rx_char;
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ INITIALIZATION                                                      │
    // │ Set up all hardware and peripherals                                │
    // └─────────────────────────────────────────────────────────────────────┘
    System_Init();
    
    // Small delay to let things stabilize
    for(volatile int i = 0; i < 1000000; i++);
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ DRAW USER INTERFACE                                                 │
    // │ Clear screen and show initial state                                │
    // └─────────────────────────────────────────────────────────────────────┘
    _USART_ClearScreen(USART2);
    UI_DrawHeader();
    UI_DrawStatus();
    UI_DrawControls();
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ MAIN LOOP                                                           │
    // │ Poll for keyboard input, button presses, and update clock          │
    // └─────────────────────────────────────────────────────────────────────┘
    while(1)
    {
        // Check for keyboard input (non-blocking)
        if (_USART_RxByte(USART2, &rx_char))
        {
            Process_KeyPress(rx_char);
        }
        
        #ifdef ENABLE_BUTTONS
        // Check for button presses (if buttons are enabled)
        Process_Buttons();
        #endif
        
        // Update uptime clock display (once per second)
        Uptime_Update();
    }
}

/*=============================================================================
 * SYSTEM INITIALIZATION
 * 
 * Sets up all hardware peripherals in the correct order:
 *   1. System clock (32MHz via PLL)
 *   2. GPIO clocks for ports A and C
 *   3. USART2 for terminal communication
 *   4. Status LED on PC6
 *   5. PWM output pin on PA4 (alternate function)
 *   6. Optional: Button inputs on PA0, PA1, PA11, PA12
 *   7. TIM14 for PWM generation
 *   8. TIM16 for 1-second uptime clock
 *===========================================================================*/

void System_Init(void)
{
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 1: Configure System Clock                                      │
    // │ Use PLL to generate 32MHz from HSI16 internal oscillator           │
    // └─────────────────────────────────────────────────────────────────────┘
    Clock_InitPll(CLOCK_CONFIG);
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 2: Enable GPIO Clocks                                          │
    // │ Must enable clock before using any GPIO pins                       │
    // └─────────────────────────────────────────────────────────────────────┘
    _GPIO_ClockEnable(GPIOA);        // Port A: PA2, PA3, PA4, and buttons
    _GPIO_ClockEnable(GPIOC);        // Port C: PC6 for status LED
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 3: Initialize USART2 for Terminal Communication               │
    // │ PA2 = TX (transmit to terminal)                                    │
    // │ PA3 = RX (receive from terminal)                                   │
    // └─────────────────────────────────────────────────────────────────────┘
    _USART_Init_USART2(SYSCLK_FREQ, BAUD_RATE);
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 4: Configure Status LED (PC6)                                 │
    // │ This LED indicates when PWM is active (ON) or stopped (OFF)        │
    // └─────────────────────────────────────────────────────────────────────┘
    _GPIO_SetPinMode(GPIOC, LED_PIN, _GPIO_PinMode_Output);
    _GPIO_PinClear(GPIOC, LED_PIN);  // Start with LED off
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 5: Configure PWM Output Pin (PA4)                             │
    // │ PA4 must be configured as Alternate Function 4 (AF4) for TIM14_CH1│
    // │ - Alternate Function mode (not regular GPIO)                       │
    // │ - AF4 = TIM14_CH1 (see datasheet table)                           │
    // │ - Push-pull output type                                            │
    // │ - High speed for clean PWM edges                                   │
    // └─────────────────────────────────────────────────────────────────────┘
    _GPIO_SetPinMode(GPIOA, PWM_PIN, _GPIO_PinMode_AlternateFunction);
    _GPIO_SetPinAlternateFunction(GPIOA, PWM_PIN, 4);  // AF4 = TIM14_CH1
    _GPIO_SetOutputType(GPIOA, PWM_PIN, _GPIO_OutputType_PushPull);
    _GPIO_SetSpeed(GPIOA, PWM_PIN, _GPIO_Speed_High);
    
    #ifdef ENABLE_BUTTONS
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 6: Configure Button Inputs (Enhancement)                      │
    // │ Buttons are active-low (pressed = 0V, released = 3.3V)            │
    // │ Internal pull-ups keep pins HIGH when button not pressed           │
    // └─────────────────────────────────────────────────────────────────────┘
    _GPIO_SetPinMode(GPIOA, BTN_S1_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S1_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S2_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S2_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S3_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S3_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S4_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S4_PIN, _GPIO_Pull_Up);
    #endif
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 7: Initialize TIM14 for PWM Generation                        │
    // │ Enable clock, configure with default frequency (100Hz)             │
    // └─────────────────────────────────────────────────────────────────────┘
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    PWM_Configure(0);  // Start with frequency index 0 (100 Hz)
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ STEP 8: Initialize TIM16 for 1-Second Uptime Clock                 │
    // │ Prescaler: 32000 → 32MHz / 32000 = 1kHz timer clock                │
    // │ Period: 1000 → 1kHz / 1000 = 1Hz = 1 second per update             │
    // └─────────────────────────────────────────────────────────────────────┘
    RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
    Timer_Init(TIM16, 32000, 1000);  // 1 second intervals at 32MHz
    Timer_Start(TIM16);
}

/*=============================================================================
 * PWM CONFIGURATION
 * 
 * Configures TIM14 for PWM generation at the specified frequency.
 * 
 * HOW IT WORKS:
 *   1. Stop the timer (to safely reconfigure)
 *   2. Set prescaler (PSC) and period (ARR) from frequency table
 *   3. Configure channel 1 for PWM Mode 1:
 *      - Output HIGH when CNT < CCR1
 *      - Output LOW when CNT >= CCR1
 *   4. Update duty cycle (sets CCR1 register)
 *   5. Enable output on channel 1
 *   6. Restart timer if PWM was previously enabled
 * 
 * PARAMETERS:
 *   freq_index: Index into FREQ_TABLE (0-4 for A-E)
 *===========================================================================*/

void PWM_Configure(uint8_t freq_index)
{
    // Validate index
    if (freq_index >= NUM_FREQUENCIES)
        return;
    
    const FrequencyConfig *config = &FREQ_TABLE[freq_index];
    
    // Stop timer before reconfiguring (safety measure)
    Timer_Stop(TIM14);
    
    // Configure timer with new frequency parameters
    // Note: Timer_Init internally subtracts 1 from PSC and ARR
    Timer_Init(TIM14, config->prescaler, config->period);
    
    // Set PWM Mode 1:
    // - Output is HIGH while counter (CNT) < compare value (CCR1)
    // - Output is LOW while counter (CNT) >= compare value (CCR1)
    Timer_ConfigPWM(TIM14, TIMER_CHANNEL1, TIMER_PWM_MODE1);
    
    // Update duty cycle to current setting
    PWM_UpdateDuty(g_state.duty_percent);
    
    // Enable channel 1 output (connects timer to PA4 pin)
    Timer_EnableOutput(TIM14, TIMER_CHANNEL1);
    
    // If PWM was running before, restart it
    if (g_state.pwm_enabled)
    {
        Timer_Start(TIM14);
    }
}

/*=============================================================================
 * UPDATE DUTY CYCLE
 * 
 * Sets the PWM duty cycle as a percentage (0-100%).
 * 
 * HOW IT WORKS:
 *   Duty cycle is controlled by the CCR1 (Capture/Compare Register 1).
 *   The formula is: Duty% = (CCR1 / ARR) × 100
 *   
 *   Example at 1kHz (ARR = 1000):
 *   - 0% duty   → CCR1 = 0     → always LOW
 *   - 25% duty  → CCR1 = 250   → HIGH for 250μs, LOW for 750μs
 *   - 50% duty  → CCR1 = 500   → HIGH for 500μs, LOW for 500μs
 *   - 100% duty → CCR1 = 1000  → always HIGH
 * 
 * PARAMETERS:
 *   percent: Duty cycle percentage (0-100), clamped automatically
 *===========================================================================*/

void PWM_UpdateDuty(uint8_t percent)
{
    // Clamp to valid range (0-100%)
    if (percent > 100)
        percent = 100;
    
    // Save new duty cycle
    g_state.duty_percent = percent;
    
    // Update CCR1 register via library function
    // This calculates: CCR1 = (ARR × percent) / 100
    Timer_SetDutyPercent(TIM14, TIMER_CHANNEL1, percent);
}

/*=============================================================================
 * TOGGLE PWM ON/OFF
 * 
 * Starts or stops PWM generation and updates status LED.
 * 
 * HOW IT WORKS:
 *   - When enabled: Timer starts counting, PWM waveform appears on PA4
 *   - When disabled: Timer stops, PA4 goes to default state (LOW)
 *   - Status LED (PC6) mirrors PWM state for visual feedback
 *===========================================================================*/

void PWM_Toggle(void)
{
    // Flip the state
    g_state.pwm_enabled = !g_state.pwm_enabled;
    
    if (g_state.pwm_enabled)
    {
        // Start PWM
        Timer_Start(TIM14);
        _GPIO_PinSet(GPIOC, LED_PIN);  // Turn LED ON
    }
    else
    {
        // Stop PWM
        Timer_Stop(TIM14);
        _GPIO_PinClear(GPIOC, LED_PIN);  // Turn LED OFF
    }
    
    // Update terminal display
    UI_Refresh();
}

/*=============================================================================
 * USER INTERFACE FUNCTIONS
 *===========================================================================*/

/*-----------------------------------------------------------------------------
 * DRAW HEADER
 * Displays the title banner at the top of the screen
 *---------------------------------------------------------------------------*/

void UI_DrawHeader(void)
{
    #ifdef ENABLE_FANCY_UI
    // Box-drawing characters (may not work on all terminals)
    _USART_TxStringXY(USART2, 1, 1, "╔══════════════════════════════════════════════════════════════════════════════╗");
    _USART_TxStringXY(USART2, 1, 2, "║               PWM FUNCTION GENERATOR - LAB 2 CMPE1250                       ║");
    _USART_TxStringXY(USART2, 1, 3, "╚══════════════════════════════════════════════════════════════════════════════╝");
    #else
    // Simple ASCII characters (works on all terminals)
    _USART_TxStringXY(USART2, 1, 1, "+==============================================================================+");
    _USART_TxStringXY(USART2, 1, 2, "|               PWM FUNCTION GENERATOR - LAB 2 CMPE1250                       |");
    _USART_TxStringXY(USART2, 1, 3, "+==============================================================================+");
    #endif
}

/*-----------------------------------------------------------------------------
 * DRAW STATUS
 * Displays current PWM state, frequency, duty cycle, and visual duty bar
 *---------------------------------------------------------------------------*/

void UI_DrawStatus(void)
{
    char buffer[100];
    const FrequencyConfig *config = &FREQ_TABLE[g_state.freq_index];
    
    #ifdef ENABLE_FANCY_UI
    _USART_TxStringXY(USART2, 1, 5, "┌─ PWM STATUS ────────────────────────────────────────────────────────────────┐");
    #else
    _USART_TxStringXY(USART2, 1, 5, "+-- PWM STATUS ---------------------------------------------------------------+");
    #endif
    
    // State line (ACTIVE or IDLE)
    sprintf(buffer, "| State:      %-15s                                              |",
            g_state.pwm_enabled ? "ACTIVE" : "IDLE (STOPPED)");
    _USART_TxStringXY(USART2, 1, 6, buffer);
    
    // Frequency line
    sprintf(buffer, "| Frequency:  %-6u Hz (Key: %c)                                            |",
            config->freq_hz, config->key);
    _USART_TxStringXY(USART2, 1, 7, buffer);
    
    // Duty cycle percentage
    sprintf(buffer, "| Duty Cycle: %3u%%                                                         |",
            g_state.duty_percent);
    _USART_TxStringXY(USART2, 1, 8, buffer);
    
    // Visual duty cycle bar (50 characters wide)
    // Each character represents 2% (50 chars × 2% = 100%)
    _USART_TxStringXY(USART2, 1, 9, "| Duty Bar:   [");
    int bars = g_state.duty_percent / 2;  // Convert 0-100% to 0-50 chars
    for (int i = 0; i < 50; i++)
    {
        if (i < bars)
            _USART_TxByte(USART2, '=');  // Filled portion
        else
            _USART_TxByte(USART2, ' ');  // Empty portion
    }
    _USART_TxString(USART2, "]       |");
    
    // Output pin information
    sprintf(buffer, "| Output Pin: PA%-2d (TIM14_CH1)                                              |", PWM_PIN);
    _USART_TxStringXY(USART2, 1, 10, buffer);
    
    #ifdef ENABLE_FANCY_UI
    _USART_TxStringXY(USART2, 1, 11, "└─────────────────────────────────────────────────────────────────────────────┘");
    #else
    _USART_TxStringXY(USART2, 1, 11, "+-----------------------------------------------------------------------------+");
    #endif
}

/*-----------------------------------------------------------------------------
 * DRAW CONTROLS
 * Displays keyboard control instructions and uptime clock
 *---------------------------------------------------------------------------*/

void UI_DrawControls(void)
{
    #ifdef ENABLE_FANCY_UI
    _USART_TxStringXY(USART2, 1, 13, "┌─ KEYBOARD CONTROLS ─────────────────────────────────────────────────────────┐");
    #else
    _USART_TxStringXY(USART2, 1, 13, "+-- KEYBOARD CONTROLS --------------------------------------------------------+");
    #endif
    
    _USART_TxStringXY(USART2, 1, 14, "| SPACE ......... Toggle PWM On/Off                                          |");
    _USART_TxStringXY(USART2, 1, 15, "| + ............. Increase Duty +5%% (clamped at 100%%)                       |");
    _USART_TxStringXY(USART2, 1, 16, "| - ............. Decrease Duty -5%% (clamped at 0%%)                         |");
    _USART_TxStringXY(USART2, 1, 17, "| 0-9 ........... Set Duty to (key x 10)%% (0%% to 90%%)                       |");
    _USART_TxStringXY(USART2, 1, 18, "|                                                                             |");
    _USART_TxStringXY(USART2, 1, 19, "| FREQUENCY SELECTION:                                                        |");
    _USART_TxStringXY(USART2, 1, 20, "|   A ........... 100 Hz      D ........... 5 kHz                            |");
    _USART_TxStringXY(USART2, 1, 21, "|   B ........... 500 Hz      E ........... 10 kHz                           |");
    _USART_TxStringXY(USART2, 1, 22, "|   C ........... 1 kHz                                                       |");
    
    #ifdef ENABLE_FANCY_UI
    _USART_TxStringXY(USART2, 1, 23, "└─────────────────────────────────────────────────────────────────────────────┘");
    #else
    _USART_TxStringXY(USART2, 1, 23, "+-----------------------------------------------------------------------------+");
    #endif
    
    // Initial clock display (00:00:00)
    _USART_TxStringXY(USART2, 1, 24, "Uptime: 00:00:00");
}

/*-----------------------------------------------------------------------------
 * REFRESH UI
 * Updates only the status section (avoids full screen redraw for less flicker)
 *---------------------------------------------------------------------------*/

void UI_Refresh(void)
{
    UI_DrawStatus();
}

/*=============================================================================
 * UPTIME CLOCK UPDATE
 * 
 * Checks TIM16 for update events (every 1 second) and updates the display.
 * 
 * HOW IT WORKS:
 *   - TIM16 is configured to overflow every 1 second
 *   - When overflow occurs, update flag (UIF) is set
 *   - We check this flag, increment counter, clear flag
 *   - Calculate hours, minutes, seconds and display
 *   - Clock wraps at 24 hours (00:00:00 to 23:59:59)
 *===========================================================================*/

void Uptime_Update(void)
{
    // Check if 1 second has elapsed
    if (Timer_CheckUpdateFlag(TIM16))
    {
        // Clear the flag (ready for next second)
        Timer_ClearUpdateFlag(TIM16);
        
        // Increment uptime counter
        g_state.uptime_seconds++;
        
        // Calculate hours, minutes, seconds with 24-hour wrap
        uint32_t hours = (g_state.uptime_seconds / 3600) % 24;  // Divide by 3600, modulo 24
        uint32_t minutes = (g_state.uptime_seconds / 60) % 60;  // Divide by 60, modulo 60
        uint32_t seconds = g_state.uptime_seconds % 60;         // Modulo 60
        
        // Format and display time (HH:MM:SS)
        char time_str[20];
        sprintf(time_str, "Uptime: %02lu:%02lu:%02lu", hours, minutes, seconds);
        _USART_TxStringXY(USART2, 1, 24, time_str);
    }
}

/*=============================================================================
 * KEYBOARD INPUT PROCESSING
 * 
 * Handles all keyboard commands from the terminal.
 * 
 * COMMANDS:
 *   SPACE - Toggle PWM on/off
 *   +/=   - Increase duty by 5%
 *   -/_   - Decrease duty by 5%
 *   0-9   - Set duty to 0%, 10%, 20%, ..., 90%
 *   A-E   - Select frequency (100Hz, 500Hz, 1kHz, 5kHz, 10kHz)
 *===========================================================================*/

void Process_KeyPress(char key)
{
    // Convert lowercase to uppercase (for frequency keys A-E)
    if (key >= 'a' && key <= 'z')
        key = key - 32;
    
    switch (key)
    {
        // ┌─────────────────────────────────────────────────────────────────┐
        // │ SPACE - Toggle PWM On/Off                                       │
        // └─────────────────────────────────────────────────────────────────┘
        case ' ':
            PWM_Toggle();
            break;
        
        // ┌─────────────────────────────────────────────────────────────────┐
        // │ + or = - Increase Duty by 5%                                    │
        // │ (Accept both to avoid needing Shift key)                       │
        // └─────────────────────────────────────────────────────────────────┘
        case '+':
        case '=':
            if (g_state.duty_percent <= 95)  // Check for overflow
            {
                PWM_UpdateDuty(g_state.duty_percent + 5);
                UI_Refresh();
            }
            break;
        
        // ┌─────────────────────────────────────────────────────────────────┐
        // │ - or _ - Decrease Duty by 5%                                    │
        // └─────────────────────────────────────────────────────────────────┘
        case '-':
        case '_':
            if (g_state.duty_percent >= 5)  // Check for underflow
            {
                PWM_UpdateDuty(g_state.duty_percent - 5);
                UI_Refresh();
            }
            break;
        
        // ┌─────────────────────────────────────────────────────────────────┐
        // │ 0-9 - Set Duty to Exact Value                                   │
        // │ 0 = 0%, 1 = 10%, 2 = 20%, ..., 9 = 90%                         │
        // └─────────────────────────────────────────────────────────────────┘
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            PWM_UpdateDuty((key - '0') * 10);  // Convert char to int, multiply by 10
            UI_Refresh();
            break;
        
        // ┌─────────────────────────────────────────────────────────────────┐
        // │ A-E - Select Frequency                                          │
        // │ Search frequency table for matching key                         │
        // └─────────────────────────────────────────────────────────────────┘
        case 'A': case 'B': case 'C': case 'D': case 'E':
            for (uint8_t i = 0; i < NUM_FREQUENCIES; i++)
            {
                if (FREQ_TABLE[i].key == key)
                {
                    g_state.freq_index = i;
                    PWM_Configure(i);
                    UI_Refresh();
                    break;
                }
            }
            break;
        
        default:
            // Ignore unrecognized keys
            break;
    }
}

#ifdef ENABLE_BUTTONS
/*=============================================================================
 * BUTTON INPUT PROCESSING (ENHANCEMENT)
 * 
 * Polls 4 buttons and processes presses with edge detection.
 * 
 * HOW IT WORKS:
 *   - Buttons are active-low (pressed = 0, released = 1)
 *   - Edge detection: Action only on falling edge (transition 1→0)
 *   - Prevents continuous triggering when button is held
 *   - Simple debouncing via polling rate limit
 * 
 * BUTTON FUNCTIONS:
 *   S1 (PA0)  - Decrease duty by 1%
 *   S2 (PA1)  - Increase duty by 1%
 *   S3 (PA11) - Previous frequency (doesn't wrap at start)
 *   S4 (PA12) - Next frequency (doesn't wrap at end)
 *===========================================================================*/

typedef struct {
    uint8_t pin;           // GPIO pin number
    uint8_t last_state;    // Previous state (for edge detection)
} ButtonState;

// Initialize button states (all released = HIGH due to pull-ups)
ButtonState buttons[4] = {
    {BTN_S1_PIN, 1},   // S1 starts released
    {BTN_S2_PIN, 1},   // S2 starts released
    {BTN_S3_PIN, 1},   // S3 starts released
    {BTN_S4_PIN, 1}    // S4 starts released
};

void Process_Buttons(void)
{
    static uint32_t counter = 0;
    
    // Simple rate limiting (poll every ~10ms at 32MHz)
    // Adjust this value if buttons are too sensitive or not responsive
    if (++counter < 10000)
        return;
    counter = 0;
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ BUTTON S1 - Decrease Duty by 1%                                     │
    // └─────────────────────────────────────────────────────────────────────┘
    uint8_t s1_state = _GPIO_GetPinIState(GPIOA, BTN_S1_PIN);
    if (s1_state == 0 && buttons[0].last_state == 1)  // Falling edge detected
    {
        if (g_state.duty_percent >= 1)  // Prevent underflow
        {
            PWM_UpdateDuty(g_state.duty_percent - 1);
            UI_Refresh();
        }
    }
    buttons[0].last_state = s1_state;
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ BUTTON S2 - Increase Duty by 1%                                     │
    // └─────────────────────────────────────────────────────────────────────┘
    uint8_t s2_state = _GPIO_GetPinIState(GPIOA, BTN_S2_PIN);
    if (s2_state == 0 && buttons[1].last_state == 1)  // Falling edge
    {
        if (g_state.duty_percent <= 99)  // Prevent overflow
        {
            PWM_UpdateDuty(g_state.duty_percent + 1);
            UI_Refresh();
        }
    }
    buttons[1].last_state = s2_state;
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ BUTTON S3 - Previous Frequency                                      │
    // │ (Doesn't wrap: stops at 100 Hz)                                    │
    // └─────────────────────────────────────────────────────────────────────┘
    uint8_t s3_state = _GPIO_GetPinIState(GPIOA, BTN_S3_PIN);
    if (s3_state == 0 && buttons[2].last_state == 1)  // Falling edge
    {
        if (g_state.freq_index > 0)  // Can't go below index 0
        {
            g_state.freq_index--;
            PWM_Configure(g_state.freq_index);
            UI_Refresh();
        }
    }
    buttons[2].last_state = s3_state;
    
    // ┌─────────────────────────────────────────────────────────────────────┐
    // │ BUTTON S4 - Next Frequency                                          │
    // │ (Doesn't wrap: stops at 10 kHz)                                    │
    // └─────────────────────────────────────────────────────────────────────┘
    uint8_t s4_state = _GPIO_GetPinIState(GPIOA, BTN_S4_PIN);
    if (s4_state == 0 && buttons[3].last_state == 1)  // Falling edge
    {
        if (g_state.freq_index < NUM_FREQUENCIES - 1)  // Can't exceed last index
        {
            g_state.freq_index++;
            PWM_Configure(g_state.freq_index);
            UI_Refresh();
        }
    }
    buttons[3].last_state = s4_state;
}
#endif  // ENABLE_BUTTONS

/*=============================================================================
 * END OF PROGRAM
 *===========================================================================*/