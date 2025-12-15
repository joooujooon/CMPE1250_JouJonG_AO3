/////////////////////////////////////////////////////////////////////////
//
//  LAB 2 - PWM FUNCTION GENERATOR
//  CMPE1250: Embedded Systems
//
//  AUTHOR: Jou Jon Galenzoga
//  FILE:   main.c
//
//  DESCRIPTION:
//    This application creates a PWM-based function generator with:
//    - 5 selectable frequencies (100Hz, 500Hz, 1kHz, 5kHz, 10kHz)
//    - Adjustable duty cycle (0% to 100% in 5% increments)
//    - Terminal UI showing current state
//    - Real-time clock display (HH:MM format)
//    - Button controls for enhancement mode
//
/////////////////////////////////////////////////////////////////////////

#include "stm32g031xx.h"
#include "clock.h"
#include "gpio.h"
#include "Timer.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////
// CONFIGURATION CONSTANTS
/////////////////////////////////////////////////////////////////////////

#define SYSCLK_FREQ     32000000U    // 32MHz system clock
#define BAUD_RATE       192300       // Terminal baud rate
#define PWM_TIMER       TIM14        // Timer for PWM generation
#define PWM_PIN         4            // PA4 - TIM14_CH1 output
#define LED_PIN         6            // PC6 - Status LED

// Button pins (for enhancement)
#define BTN_S1_PIN      0            // PA0
#define BTN_S2_PIN      1            // PA1
#define BTN_S3_PIN      11           // PA11
#define BTN_S4_PIN      12           // PA12

/////////////////////////////////////////////////////////////////////////
// FREQUENCY TABLE
/////////////////////////////////////////////////////////////////////////

typedef struct {
    char key;           // Keyboard key
    uint16_t freq_hz;   // Frequency in Hz
    uint16_t prescaler; // Timer prescaler
    uint16_t period;    // Timer period (ARR)
} FrequencyConfig;

// Frequency lookup table
// Formula: PWM_freq = SYSCLK / (PSC × ARR)
// We want clean divisors for 32MHz
const FrequencyConfig FREQ_TABLE[] = {
    {'A', 100,   320, 1000},  // 32MHz/(320×1000) = 100Hz
    {'B', 500,   64,  1000},  // 32MHz/(64×1000)  = 500Hz
    {'C', 1000,  32,  1000},  // 32MHz/(32×1000)  = 1kHz
    {'D', 5000,  64,  100},   // 32MHz/(64×100)   = 5kHz
    {'E', 10000, 32,  100}    // 32MHz/(32×100)   = 10kHz
};

#define NUM_FREQUENCIES (sizeof(FREQ_TABLE) / sizeof(FrequencyConfig))

/////////////////////////////////////////////////////////////////////////
// GLOBAL STATE VARIABLES
/////////////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t freq_index;      // Current frequency index (0-4)
    uint8_t duty_percent;    // Current duty cycle (0-100)
    uint8_t pwm_enabled;     // PWM on/off state
    uint32_t uptime_seconds; // Application uptime
} AppState;

AppState g_state = {
    .freq_index = 0,      // Start at 100Hz
    .duty_percent = 50,   // Start at 50% duty
    .pwm_enabled = 0,     // Start with PWM off
    .uptime_seconds = 0
};

/////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
/////////////////////////////////////////////////////////////////////////

void System_Init(void);
void PWM_Configure(uint8_t freq_index);
void PWM_UpdateDuty(uint8_t percent);
void PWM_Toggle(void);
void UI_Refresh(void);
void UI_DrawHeader(void);
void UI_DrawStatus(void);
void UI_DrawControls(void);
void Uptime_Update(void);
void Process_KeyPress(char key);
void Process_Buttons(void);

/////////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
/////////////////////////////////////////////////////////////////////////

int main(void)
{
    char rx_char;
    
    // Initialize all subsystems
    System_Init();
    
    // Show initial UI
    _USART_ClearScreen(USART2);
    UI_DrawHeader();
    UI_DrawStatus();
    UI_DrawControls();
    
    while (1)
    {
        // Check for keyboard input (non-blocking)
        if (_USART_RxByte(USART2, &rx_char))
        {
            Process_KeyPress(rx_char);
        }
        
        // Check for button presses (enhancement)
        Process_Buttons();
        
        // Update clock display every second
        Uptime_Update();
    }
}

/////////////////////////////////////////////////////////////////////////
// INITIALIZATION FUNCTIONS
/////////////////////////////////////////////////////////////////////////

void System_Init(void)
{
    // 1. Configure system clock to 32MHz
    Clock_InitPll(PLL_32MHZ);
    
    // 2. Enable GPIO clocks
    _GPIO_ClockEnable(GPIOA);
    _GPIO_ClockEnable(GPIOC);
    
    // 3. Initialize USART2 for terminal
    _USART_Init_USART2(SYSCLK_FREQ, BAUD_RATE);
    
    // 4. Configure LED pin (PC6)
    _GPIO_SetPinMode(GPIOC, LED_PIN, _GPIO_PinMode_Output);
    _GPIO_SetOutputType(GPIOC, LED_PIN, _GPIO_OutputType_PushPull);
    _GPIO_PinClear(GPIOC, LED_PIN);  // LED off initially
    
    // 5. Configure PWM output pin (PA4 - TIM14_CH1)
    _GPIO_SetPinMode(GPIOA, PWM_PIN, _GPIO_PinMode_AlternateFunction);
    _GPIO_SetPinAlternateFunction(GPIOA, PWM_PIN, 4);  // AF4 = TIM14_CH1
    _GPIO_SetOutputType(GPIOA, PWM_PIN, _GPIO_OutputType_PushPull);
    _GPIO_SetSpeed(GPIOA, PWM_PIN, _GPIO_Speed_High);
    
    // 6. Enable TIM14 clock
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    
    // 7. Configure buttons (enhancement) - with pull-ups
    _GPIO_SetPinMode(GPIOA, BTN_S1_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S1_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S2_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S2_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S3_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S3_PIN, _GPIO_Pull_Up);
    
    _GPIO_SetPinMode(GPIOA, BTN_S4_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S4_PIN, _GPIO_Pull_Up);
    
    // 8. Initialize PWM with default frequency (100Hz)
    PWM_Configure(0);
    
    // 9. Set default duty cycle
    PWM_UpdateDuty(g_state.duty_percent);
    
    // 10. Setup a timer for uptime clock (using TIM16)
    RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
    Timer_Init(TIM16, 32000, 1000);  // 1 second intervals at 32MHz
    Timer_Start(TIM16);
}

/////////////////////////////////////////////////////////////////////////
// PWM CONTROL FUNCTIONS
/////////////////////////////////////////////////////////////////////////

void PWM_Configure(uint8_t freq_index)
{
    if (freq_index >= NUM_FREQUENCIES)
        return;
    
    const FrequencyConfig *config = &FREQ_TABLE[freq_index];
    
    // Stop timer during reconfiguration
    Timer_Stop(PWM_TIMER);
    
    // Configure timer with new prescaler and period
    Timer_Init(PWM_TIMER, config->prescaler, config->period);
    
    // Configure PWM mode
    Timer_ConfigPWM(PWM_TIMER, TIMER_CHANNEL1, TIMER_PWM_MODE1);
    
    // Update duty cycle
    PWM_UpdateDuty(g_state.duty_percent);
    
    // Enable output
    Timer_EnableOutput(PWM_TIMER, TIMER_CHANNEL1);
    
    // Restart timer if PWM was enabled
    if (g_state.pwm_enabled)
    {
        Timer_Start(PWM_TIMER);
    }
}

void PWM_UpdateDuty(uint8_t percent)
{
    // Clamp to 0-100%
    if (percent > 100)
        percent = 100;
    
    g_state.duty_percent = percent;
    Timer_SetDutyPercent(PWM_TIMER, TIMER_CHANNEL1, percent);
}

void PWM_Toggle(void)
{
    g_state.pwm_enabled = !g_state.pwm_enabled;
    
    if (g_state.pwm_enabled)
    {
        Timer_Start(PWM_TIMER);
        _GPIO_PinSet(GPIOC, LED_PIN);  // Turn on LED
    }
    else
    {
        Timer_Stop(PWM_TIMER);
        _GPIO_PinClear(GPIOC, LED_PIN);  // Turn off LED
    }
    
    UI_Refresh();
}

/////////////////////////////////////////////////////////////////////////
// USER INTERFACE FUNCTIONS
/////////////////////////////////////////////////////////////////////////

void UI_DrawHeader(void)
{
    _USART_TxStringXY(USART2, 1, 1, "╔══════════════════════════════════════════════════════════════════════════════╗");
    _USART_TxStringXY(USART2, 1, 2, "║               PWM FUNCTION GENERATOR - LAB 2 CMPE1250                       ║");
    _USART_TxStringXY(USART2, 1, 3, "╚══════════════════════════════════════════════════════════════════════════════╝");
}

void UI_DrawStatus(void)
{
    char buffer[100];
    const FrequencyConfig *config = &FREQ_TABLE[g_state.freq_index];
    
    // PWM Status line
    _USART_TxStringXY(USART2, 1, 5, "┌─ PWM STATUS ────────────────────────────────────────────────────────────────┐");
    
    sprintf(buffer, "│ State:      %-15s", g_state.pwm_enabled ? "ACTIVE" : "IDLE (STOPPED)");
    _USART_TxStringXY(USART2, 1, 6, buffer);
    _USART_TxStringXY(USART2, 65, 6, "              │");
    
    sprintf(buffer, "│ Frequency:  %-6u Hz (Key: %c)", config->freq_hz, config->key);
    _USART_TxStringXY(USART2, 1, 7, buffer);
    _USART_TxStringXY(USART2, 65, 7, "              │");
    
    sprintf(buffer, "│ Duty Cycle: %3u%%", g_state.duty_percent);
    _USART_TxStringXY(USART2, 1, 8, buffer);
    _USART_TxStringXY(USART2, 65, 8, "              │");
    
    // Draw duty cycle bar
    _USART_TxStringXY(USART2, 1, 9, "│ Duty Bar:   [");
    int bars = g_state.duty_percent / 2;  // 50 chars = 100%
    for (int i = 0; i < 50; i++)
    {
        if (i < bars)
            _USART_TxByte(USART2, '█');
        else
            _USART_TxByte(USART2, '░');
    }
    _USART_TxStringXY(USART2, 66, 9, "]         │");
    
    sprintf(buffer, "│ Output Pin: PA%-2d (TIM14_CH1)", PWM_PIN);
    _USART_TxStringXY(USART2, 1, 10, buffer);
    _USART_TxStringXY(USART2, 65, 10, "              │");
    
    _USART_TxStringXY(USART2, 1, 11, "└─────────────────────────────────────────────────────────────────────────────┘");
}

void UI_DrawControls(void)
{
    _USART_TxStringXY(USART2, 1, 13, "┌─ KEYBOARD CONTROLS ─────────────────────────────────────────────────────────┐");
    _USART_TxStringXY(USART2, 1, 14, "│ SPACE ......... Toggle PWM On/Off                                           │");
    _USART_TxStringXY(USART2, 1, 15, "│ + ............. Increase Duty +5% (clamped at 100%)                         │");
    _USART_TxStringXY(USART2, 1, 16, "│ - ............. Decrease Duty -5% (clamped at 0%)                           │");
    _USART_TxStringXY(USART2, 1, 17, "│ 0-9 ........... Set Duty to (key × 10)% (0% to 90%)                         │");
    _USART_TxStringXY(USART2, 1, 18, "│                                                                              │");
    _USART_TxStringXY(USART2, 1, 19, "│ FREQUENCY SELECTION:                                                         │");
    _USART_TxStringXY(USART2, 1, 20, "│   A ........... 100 Hz      D ........... 5 kHz                             │");
    _USART_TxStringXY(USART2, 1, 21, "│   B ........... 500 Hz      E ........... 10 kHz                            │");
    _USART_TxStringXY(USART2, 1, 22, "│   C ........... 1 kHz                                                        │");
    _USART_TxStringXY(USART2, 1, 23, "└─────────────────────────────────────────────────────────────────────────────┘");
    
    // Clock display
    _USART_TxStringXY(USART2, 1, 24, "Uptime: 00:00:00");
}

void UI_Refresh(void)
{
    // Only redraw the status section to avoid flicker
    UI_DrawStatus();
}

/////////////////////////////////////////////////////////////////////////
// UPTIME CLOCK FUNCTION
/////////////////////////////////////////////////////////////////////////

void Uptime_Update(void)
{
    // Check if 1 second has elapsed
    if (Timer_CheckUpdateFlag(TIM16))
    {
        Timer_ClearUpdateFlag(TIM16);
        g_state.uptime_seconds++;
        
        // Calculate hours, minutes, seconds
        uint32_t hours = (g_state.uptime_seconds / 3600) % 24;
        uint32_t minutes = (g_state.uptime_seconds / 60) % 60;
        uint32_t seconds = g_state.uptime_seconds % 60;
        
        // Update clock display
        char time_str[20];
        sprintf(time_str, "Uptime: %02lu:%02lu:%02lu", hours, minutes, seconds);
        _USART_TxStringXY(USART2, 1, 24, time_str);
    }
}

/////////////////////////////////////////////////////////////////////////
// INPUT PROCESSING FUNCTIONS
/////////////////////////////////////////////////////////////////////////

void Process_KeyPress(char key)
{
    // Convert to uppercase for frequency keys
    if (key >= 'a' && key <= 'z')
        key = key - 32;
    
    switch (key)
    {
        case ' ':  // Space - Toggle PWM
            PWM_Toggle();
            break;
            
        case '+':  // Increase duty by 5%
        case '=':  // Also accept = key (shift not needed)
            if (g_state.duty_percent <= 95)
            {
                PWM_UpdateDuty(g_state.duty_percent + 5);
                UI_Refresh();
            }
            break;
            
        case '-':  // Decrease duty by 5%
        case '_':
            if (g_state.duty_percent >= 5)
            {
                PWM_UpdateDuty(g_state.duty_percent - 5);
                UI_Refresh();
            }
            break;
            
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            // Set duty to (digit × 10)%
            PWM_UpdateDuty((key - '0') * 10);
            UI_Refresh();
            break;
            
        case 'A':  // 100 Hz
        case 'B':  // 500 Hz
        case 'C':  // 1 kHz
        case 'D':  // 5 kHz
        case 'E':  // 10 kHz
            // Find frequency index
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
            // Ignore unknown keys
            break;
    }
}

/////////////////////////////////////////////////////////////////////////
// ENHANCEMENT: BUTTON PROCESSING
/////////////////////////////////////////////////////////////////////////

// Simple debouncing state machine
typedef struct {
    uint8_t pin;
    uint8_t last_state;
    uint8_t stable_count;
} ButtonState;

ButtonState buttons[4] = {
    {BTN_S1_PIN, 1, 0},
    {BTN_S2_PIN, 1, 0},
    {BTN_S3_PIN, 1, 0},
    {BTN_S4_PIN, 1, 0}
};

#define DEBOUNCE_THRESHOLD 5

void Process_Buttons(void)
{
    static uint32_t last_check = 0;
    static uint8_t counter = 0;
    
    // Simple delay (non-blocking)
    if (++counter < 10000)
        return;
    counter = 0;
    
    // S1 - Duty -1%
    uint8_t s1_state = _GPIO_GetPinIState(GPIOA, BTN_S1_PIN);
    if (s1_state == 0 && buttons[0].last_state == 1)  // Falling edge
    {
        if (g_state.duty_percent >= 1)
        {
            PWM_UpdateDuty(g_state.duty_percent - 1);
            UI_Refresh();
        }
    }
    buttons[0].last_state = s1_state;
    
    // S2 - Duty +1%
    uint8_t s2_state = _GPIO_GetPinIState(GPIOA, BTN_S2_PIN);
    if (s2_state == 0 && buttons[1].last_state == 1)
    {
        if (g_state.duty_percent <= 99)
        {
            PWM_UpdateDuty(g_state.duty_percent + 1);
            UI_Refresh();
        }
    }
    buttons[1].last_state = s2_state;
    
    // S3 - Previous frequency
    uint8_t s3_state = _GPIO_GetPinIState(GPIOA, BTN_S3_PIN);
    if (s3_state == 0 && buttons[2].last_state == 1)
    {
        if (g_state.freq_index > 0)
        {
            g_state.freq_index--;
            PWM_Configure(g_state.freq_index);
            UI_Refresh();
        }
    }
    buttons[2].last_state = s3_state;
    
    // S4 - Next frequency
    uint8_t s4_state = _GPIO_GetPinIState(GPIOA, BTN_S4_PIN);
    if (s4_state == 0 && buttons[3].last_state == 1)
    {
        if (g_state.freq_index < NUM_FREQUENCIES - 1)
        {
            g_state.freq_index++;
            PWM_Configure(g_state.freq_index);
            UI_Refresh();
        }
    }
    buttons[3].last_state = s4_state;
}
