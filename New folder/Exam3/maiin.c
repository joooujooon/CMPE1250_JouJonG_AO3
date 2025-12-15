
#include "stm32g031xx.h"
#include "clock.h"
#include "gpio.h"
#include "Timer.h"

/*=============================================================================
 * CONFIGURATION
 *===========================================================================*/

// Part A: 50MHz System Clock
#define SYSCLK_FREQ         64000000U    // 64 MHz
#define CLOCK_CONFIG        PLL_64MHZ    // Use 64MHz enum, will adjust manually

// Part B: PWM Configuration for 10kHz, 25% duty
#define PWM_FREQ_HZ         10000        // 10 kHz
#define PWM_INITIAL_DUTY    25           // 25% initial duty

// Part C: Duty cycle 
#define DUTY_MIN            10           // Minimum duty: 10%
#define DUTY_MAX            90           // Maximum duty: 90%
#define DUTY_DECREASE_STEP  2            // S1 decreases by 2%
#define DUTY_INCREASE_STEP  7            // S2 increases by 7%

// Pin definitions
#define PWM_PIN             4            // PA4 - TIM14_CH1
#define MCO_PIN             7            // PA7 - MCO output
#define BTN_S1_PIN          0            // PA0 - Duty -2%
#define BTN_S2_PIN          1            // PA1 - Duty +7%



typedef struct {
    uint8_t duty_percent;      
    uint8_t s1_last_state;    
    uint8_t s2_last_state;    
} AppState;

AppState g_state = {
    .duty_percent = PWM_INITIAL_DUTY,   // Start at 25%
    .s1_last_state = 1,                  // Buttons released (pull-up = HIGH)
    .s2_last_state = 1
};


 //* FUNCTION PROTOTYPES


void System_Init(void);
void Clock_Init_50MHz(void);
void MCO_Init(void);
void TIM16_Init_20ms(void);
void PWM_Init_10kHz_25Percent(void);
void PWM_UpdateDuty(uint8_t percent);
void Process_Buttons(void);

//* MAIN FUNCTION


int main(void)
{
    // Initialize all hardware
    System_Init();
    
    // Main loop: Poll buttons and TIM16 update events
    while(1)
    {
       
        Process_Buttons();
        
        
        if (Timer_CheckUpdateFlag(TIM16))
        {
            Timer_ClearUpdateFlag(TIM16);
            
        }
    }
}

/*=============================================================================
 * SYSTEM INITIALIZATION
 *  Parts A, B, and C
 *===========================================================================*/

void System_Init(void)
{
   
    Clock_Init_50MHz();
    

    MCO_Init();
    

     TIM16_Init_20ms();
    
    PWM_Init_10kHz_25Percent();
    

    _GPIO_ClockEnable(GPIOA);
    
    // S1 (PA0) - Decrease duty by 2%
    _GPIO_SetPinMode(GPIOA, BTN_S1_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S1_PIN, _GPIO_Pull_Up);
    
    // S2 (PA1) - Increase duty by 7%
    _GPIO_SetPinMode(GPIOA, BTN_S2_PIN, _GPIO_PinMode_Input);
    _GPIO_SetPull(GPIOA, BTN_S2_PIN, _GPIO_Pull_Up);
}


// * PART A: CLOCK INITIALIZATION - 64MHZ

void Clock_Init_64MHz(void)
{
    // Enable HSI16 and wait for it to be ready
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    
 
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= (2 << FLASH_ACR_LATENCY_Pos);  // 2 wait states
    while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != (2 << FLASH_ACR_LATENCY_Pos));
    
    // Disable PLL before configuration
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY);
    
    // Configure PLL: M=2, N=25, R=4
    // PLLM = /2 (bits = 1 for divide by 2)
    // PLLN = ×25 (bits = 25)
    // PLLR = /4 (bits = 01 for divide by 4)
    RCC->PLLCFGR = 0;  // Clear register
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;              // Source = HSI16
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLM_Pos);         // M = /2
    RCC->PLLCFGR |= (25 << RCC_PLLCFGR_PLLN_Pos);        // N = ×25
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLR_Pos);         // R = /4 (01 binary)
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;                  // Enable PLLR output
    
    // Enable PLL and wait for lock
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    
    // Switch system clock to PLL
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= (3 << RCC_CFGR_SW_Pos);  // 0b11 = PLL
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (3 << RCC_CFGR_SWS_Pos));
    
    // Update CMSIS system clock variable
    SystemCoreClockUpdate();
}


void MCO_Init(void)
{
    // Enable GPIOA clock
    _GPIO_ClockEnable(GPIOA);
    
    // Configure PA7 as Alternate Function (AF0 = MCO)
    _GPIO_SetPinMode(GPIOA, MCO_PIN, _GPIO_PinMode_AlternateFunction);
    _GPIO_SetPinAlternateFunction(GPIOA, MCO_PIN, 0);  // AF0 = MCO
    _GPIO_SetOutputType(GPIOA, MCO_PIN, _GPIO_OutputType_PushPull);
    _GPIO_SetSpeed(GPIOA, MCO_PIN, _GPIO_Speed_VeryHigh);
    
    // Configure MCO to output SYSCLK divided by 16
    // This gives 50MHz / 16 = 3.125 MHz on oscilloscope (easier to measure)
    // Change divider as needed for your scope
    
    // MCO source selection: SYSCLK
    GPIO_InitOutput(GPIOA, 0); // PA5 For Oscilloscope
    GPIO_InitOutput(GPIOC, 6); // PC6

    
    // Note: If you want full 50MHz on PA7, use MCO_Div_1
    // But this may be too fast for some oscilloscopes to trigger reliably
}


void TIM16_Init_20ms(void)
{
    // Enable TIM16 clock
    RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
    
    // Configure timer for 20ms (50 Hz) update events
    // PSC = 1000 → 50MHz / 1000 = 50kHz timer clock
    // ARR = 1000 → 50kHz / 1000 = 50Hz = 20ms period
    Timer_Init(TIM16, 1000, 1000);
    
    // Start the timer
    Timer_Start(TIM16);
}

/*=============================================================================
 * PART B: PWM INITIALIZATION - 10kHz, 25% Duty
 * 
 * Configures TIM14 to produce PWM on channel 1 (PA4).
 * Initial settings: 10kHz frequency, 25% duty cycle
 *===========================================================================*/

void PWM_Init_10kHz_25Percent(void)
{
    // Enable GPIOA clock (if not already enabled)
    _GPIO_ClockEnable(GPIOA);
    
    // Configure PA4 as Alternate Function 4 (TIM14_CH1)
    _GPIO_SetPinMode(GPIOA, PWM_PIN, _GPIO_PinMode_AlternateFunction);
    _GPIO_SetPinAlternateFunction(GPIOA, PWM_PIN, 4);  // AF4 = TIM14_CH1
    _GPIO_SetOutputType(GPIOA, PWM_PIN, _GPIO_OutputType_PushPull);
    _GPIO_SetSpeed(GPIOA, PWM_PIN, _GPIO_Speed_High);
    
    // Enable TIM14 clock
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    
    // Initialize timer for 10kHz
    // PSC = 5 → 50MHz / 5 = 10MHz timer clock
    // ARR = 1000 → 10MHz / 1000 = 10kHz PWM frequency
    Timer_Init(TIM14, 5, 1000);
    
    // Configure PWM Mode 1 on Channel 1
    Timer_ConfigPWM(TIM14, TIMER_CHANNEL1, TIMER_PWM_MODE1);
    
    // Set initial duty cycle to 25%
    PWM_UpdateDuty(PWM_INITIAL_DUTY);
    
    // Enable channel 1 output
    Timer_EnableOutput(TIM14, TIMER_CHANNEL1);
    
    // Start PWM generation
    Timer_Start(TIM14);
}

/*=============================================================================
 * PART B/C: UPDATE PWM DUTY CYCLE
 * 
 * the PWM duty cycle with clamping to 10%-90% range.
 * 
 *===========================================================================*/

void PWM_UpdateDuty(uint8_t percent)
{
    // Part C: Clamp duty cycle to 10%-90% range
    if (percent < DUTY_MIN)
        percent = DUTY_MIN;
    if (percent > DUTY_MAX)
        percent = DUTY_MAX;
    
    // Save new duty
    g_state.duty_percent = percent;
    
    // Update CCR1 via library function
    Timer_SetDutyPercent(TIM14, TIMER_CHANNEL1, percent);
}

// * PART C:
 * 
 //buttons S1 and S2 for transitions (edge detection).
 * 

//void Process_Buttons(void)
//{
//    static uint32_t debounce_counter = 0;
    
//    // Simple debouncing via polling rate limit
//    // Adjust this counter if buttons are too sensitive or not responsive
//    if (++debounce_counter < 5000)
//        return;
//    debounce_counter = 0;
    
//   //button 1

//    uint8_t s1_current = _GPIO_GetPinIState(GPIOA, BTN_S1_PIN);
    
//    // Check for falling edge (released → pressed: 1 → 0)
//    if (s1_current == 0 && g_state.s1_last_state == 1)
//    {
//        // Button S1 was just pressed
//        // Decrease duty by 2% (will be clamped to minimum 10%)
//        PWM_UpdateDuty(g_state.duty_percent - DUTY_DECREASE_STEP);
//    }
    
//    // Save current state for next iteration
//    g_state.s1_last_state = s1_current;
    
//    // Button 2
//    uint8_t s2_current = _GPIO_GetPinIState(GPIOA, BTN_S2_PIN);
    
//    // Check for falling edge (released → pressed: 1 → 0)
//    if (s2_current == 0 && g_state.s2_last_state == 1)
//    {
//        // Button S2 was just pressed
//        // Increase duty by 7% (will be clamped to maximum 90%)
//        PWM_UpdateDuty(g_state.duty_percent + DUTY_INCREASE_STEP);
//    }
    
//    // Save current state for next iteration
//    g_state.s2_last_state = s2_current;
//}
