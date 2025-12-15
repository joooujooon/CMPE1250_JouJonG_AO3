# LAB 2: PWM FUNCTION GENERATOR - STEP-BY-STEP GUIDE

## 📋 Overview
This lab creates a PWM-based function generator that outputs configurable square waves on PA4 (TIM14_CH1). The application provides a terminal UI for control and monitoring.

---

## 🎯 Requirements Summary

### Base Requirements (70%)
1. ✅ Configure system clock to 32MHz
2. ✅ Configure PWM on a timer pin (PA4 - TIM14_CH1)
3. ✅ Terminal UI at 192300 baud showing:
   - PWM state (on/off)
   - Current frequency
   - Current duty cycle
   - Visual duty cycle bar
4. ✅ Controls via keyboard
5. ✅ Real-time clock display (HH:MM:SS)
6. ✅ Status LED (PC6) indicates PWM state

### Enhancement (30%)
1. ✅ Button controls for fine adjustments
2. ✅ 4 buttons: Duty±1%, Frequency up/down

---

## 🔧 Hardware Connections

### Required Pins
```
PA4  - PWM Output (TIM14_CH1)    → Connect to oscilloscope/LED
PC6  - Status LED                 → Shows when PWM is active
PA2  - USART2 TX                  → Terminal
PA3  - USART2 RX                  → Terminal

Enhancement (Optional):
PA0  - Button S1 (Duty -1%)
PA1  - Button S2 (Duty +1%)
PA11 - Button S3 (Prev Freq)
PA12 - Button S4 (Next Freq)
```

---

## 📝 Step-by-Step Implementation

### STEP 1: Clock Configuration (32MHz)

**Why 32MHz?**
- Easy to divide for clean PWM frequencies
- Fast enough for high-frequency PWM
- Within STM32G031 specifications

**Math for Clock:**
```
Target SYSCLK = 32MHz
Using HSI16 (16MHz internal oscillator)
PLL Formula: SYSCLK = HSI16 × N / R

For 32MHz:
  N = 8 (multiply by 8)
  R = 2 (divide by 2)
  Result: 16MHz × 8 / 2 = 64MHz... wait, that's wrong!

Correct calculation:
  N = 8, R = 4
  Result: 16MHz × 8 / 4 = 32MHz ✓
```

**Code:**
```c
Clock_InitPll(PLL_32MHZ);  // Uses your clock library
```

**Your clock.c already handles this!** It calculates N and R automatically.

---

### STEP 2: GPIO Configuration

**PA4 - PWM Output Pin**

Why PA4?
- PA4 supports TIM14_CH1 on AF4 (Alternate Function 4)
- Good for PWM output

```c
_GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_AlternateFunction);
_GPIO_SetPinAlternateFunction(GPIOA, 4, 4);  // AF4 = TIM14_CH1
_GPIO_SetOutputType(GPIOA, 4, _GPIO_OutputType_PushPull);
_GPIO_SetSpeed(GPIOA, 4, _GPIO_Speed_High);
```

**PC6 - Status LED**
```c
_GPIO_SetPinMode(GPIOC, 6, _GPIO_PinMode_Output);
_GPIO_SetOutputType(GPIOC, 6, _GPIO_OutputType_PushPull);
```

**Buttons (Enhancement)**
```c
_GPIO_SetPinMode(GPIOA, 0, _GPIO_PinMode_Input);
_GPIO_SetPull(GPIOA, 0, _GPIO_Pull_Up);  // Active-low with pull-up
```

---

### STEP 3: PWM Frequency Calculations

**Understanding PWM Frequency:**
```
PWM_Frequency = SYSCLK / (Prescaler × Period)

For our 32MHz system:
```

| Frequency | Math                      | PSC  | ARR  |
|-----------|---------------------------|------|------|
| 100 Hz    | 32MHz / (320 × 1000)      | 320  | 1000 |
| 500 Hz    | 32MHz / (64 × 1000)       | 64   | 1000 |
| 1 kHz     | 32MHz / (32 × 1000)       | 32   | 1000 |
| 5 kHz     | 32MHz / (64 × 100)        | 64   | 100  |
| 10 kHz    | 32MHz / (32 × 100)        | 32   | 100  |

**Why these values?**
- Larger ARR (1000) = finer duty cycle resolution at low frequencies
- Smaller ARR (100) = coarser resolution but achieves higher frequencies
- PSC and ARR chosen to divide evenly into 32MHz

**Verification:**
```
100 Hz: 32,000,000 / (320 × 1000) = 100 ✓
  5 kHz: 32,000,000 / (64 × 100) = 5,000 ✓
```

---

### STEP 4: Timer Configuration for PWM

**Code Flow:**
```c
// 1. Enable TIM14 clock
RCC->APBENR2 |= RCC_APBENR2_TIM14EN;

// 2. Configure timer (using your library)
Timer_Init(TIM14, prescaler, period);

// 3. Configure PWM Mode 1
Timer_ConfigPWM(TIM14, TIMER_CHANNEL1, TIMER_PWM_MODE1);

// 4. Set duty cycle
Timer_SetDutyPercent(TIM14, TIMER_CHANNEL1, 50);  // 50%

// 5. Enable output
Timer_EnableOutput(TIM14, TIMER_CHANNEL1);

// 6. Start timer
Timer_Start(TIM14);
```

**What's happening in hardware:**

PWM Mode 1 means:
- Output is HIGH when CNT < CCR
- Output is LOW when CNT >= CCR

```
Timer Counter (CNT) counts: 0 → ARR → 0 → ARR ...

If ARR = 1000 and CCR = 500 (50% duty):
  CNT: 0→500   → Output HIGH
  CNT: 500→1000 → Output LOW
  Repeat...
```

---

### STEP 5: Duty Cycle Control

**Understanding Duty Cycle:**
```
Duty% = (CCR / ARR) × 100%

For 50% duty with ARR=1000:
  CCR = 500
  Duty = (500/1000) × 100% = 50%

For 75% duty:
  CCR = 750
  Duty = (750/1000) × 100% = 75%
```

**Your library function:**
```c
void Timer_SetDutyPercent(TIM_TypeDef *pTimer, Timer_Channel channel, uint8_t percent)
{
    if (percent > 100) percent = 100;
    
    uint32_t arr = pTimer->ARR + 1;  // ARR is 0-indexed
    uint16_t duty = (arr * percent) / 100;
    
    pTimer->CCR1 = duty;  // Set compare value
}
```

**Example:**
```
ARR = 1000 (actually 999 in register)
Percent = 75%

duty = (1000 × 75) / 100 = 750
CCR1 = 750

Result: 75% duty cycle!
```

---

### STEP 6: Terminal UI Design

**Screen Layout:**
```
┌──────────────────────────────────────────────────────────────────────────────┐
│               PWM FUNCTION GENERATOR - LAB 2 CMPE1250                       │
└──────────────────────────────────────────────────────────────────────────────┘

┌─ PWM STATUS ────────────────────────────────────────────────────────────────┐
│ State:      ACTIVE                                                          │
│ Frequency:  1000 Hz (Key: C)                                                │
│ Duty Cycle: 50%                                                             │
│ Duty Bar:   [█████████████████████████░░░░░░░░░░░░░░░░░░░░░░░░░]           │
│ Output Pin: PA4 (TIM14_CH1)                                                 │
└─────────────────────────────────────────────────────────────────────────────┘

┌─ KEYBOARD CONTROLS ─────────────────────────────────────────────────────────┐
│ SPACE ......... Toggle PWM On/Off                                           │
│ + ............. Increase Duty +5%                                           │
│ - ............. Decrease Duty -5%                                           │
│ 0-9 ........... Set Duty to (key × 10)%                                     │
│                                                                              │
│ FREQUENCY SELECTION:                                                         │
│   A ........... 100 Hz      D ........... 5 kHz                             │
│   B ........... 500 Hz      E ........... 10 kHz                            │
│   C ........... 1 kHz                                                        │
└─────────────────────────────────────────────────────────────────────────────┘

Uptime: 00:12:34
```

**Implementation:**
```c
// Use ANSI escape codes via your USART library
_USART_GotoXY(USART2, col, row);        // Position cursor
_USART_TxString(USART2, "text");        // Print text
_USART_ClearScreen(USART2);             // Clear screen
```

---

### STEP 7: Keyboard Input Processing

**Input Mapping:**
```c
void Process_KeyPress(char key)
{
    switch (key)
    {
        case ' ':  // Toggle PWM
            PWM_Toggle();
            break;
            
        case '+':  // Increase duty +5%
            PWM_UpdateDuty(duty + 5);
            break;
            
        case '-':  // Decrease duty -5%
            PWM_UpdateDuty(duty - 5);
            break;
            
        case '0'-'9':  // Set duty to (key × 10)%
            PWM_UpdateDuty((key - '0') * 10);
            break;
            
        case 'A': case 'B': case 'C': case 'D': case 'E':
            // Change frequency
            PWM_Configure(freq_index);
            break;
    }
}
```

**Non-blocking input:**
```c
char rx_char;
if (_USART_RxByte(USART2, &rx_char))  // Only processes if char available
{
    Process_KeyPress(rx_char);
}
```

---

### STEP 8: Uptime Clock

**Using TIM16 for 1-second interrupts:**

```c
// Configure TIM16 for 1Hz updates
RCC->APBENR2 |= RCC_APBENR2_TIM16EN;

// For 32MHz: PSC=32000, ARR=1000 → 1 second
Timer_Init(TIM16, 32000, 1000);
Timer_Start(TIM16);
```

**Clock update in main loop:**
```c
void Uptime_Update(void)
{
    if (Timer_CheckUpdateFlag(TIM16))
    {
        Timer_ClearUpdateFlag(TIM16);
        uptime_seconds++;
        
        // Calculate HH:MM:SS
        uint32_t hours = (uptime_seconds / 3600) % 24;
        uint32_t minutes = (uptime_seconds / 60) % 60;
        uint32_t seconds = uptime_seconds % 60;
        
        // Update display
        sprintf(time_str, "Uptime: %02lu:%02lu:%02lu", hours, minutes, seconds);
        _USART_TxStringXY(USART2, 1, 24, time_str);
    }
}
```

---

### STEP 9: Enhancement - Button Controls

**Button Debouncing:**
```c
// Simple edge detection
uint8_t button_state = _GPIO_GetPinIState(GPIOA, BTN_PIN);

if (button_state == 0 && last_state == 1)  // Falling edge (pressed)
{
    // Button action here
}

last_state = button_state;
```

**Button Functions:**
- S1 (PA0): Duty -1%
- S2 (PA1): Duty +1%
- S3 (PA11): Previous frequency
- S4 (PA12): Next frequency

---

## 🧮 Important Formulas Reference

### Clock Math
```
SYSCLK = HSI16 × N / R
32MHz = 16MHz × 8 / 4
```

### PWM Frequency
```
PWM_freq = SYSCLK / (PSC × ARR)
100 Hz = 32MHz / (320 × 1000)
```

### Duty Cycle
```
Duty% = (CCR / ARR) × 100%
50% = (500 / 1000) × 100%
```

### Timer Calculations
```
Timer_Update_Freq = SYSCLK / (PSC × ARR)

For 1 second tick at 32MHz:
PSC = 32000, ARR = 1000
Freq = 32MHz / (32000 × 1000) = 1 Hz ✓
```

---

## 🎓 Key Concepts Explained

### 1. Why Alternate Function?
PA4 can be a regular GPIO OR a timer output. To use it for PWM, we configure it as "Alternate Function" and specify AF4 (which routes TIM14_CH1 to PA4).

### 2. Why Push-Pull Output?
Push-pull can actively drive HIGH and LOW. Open-drain can only pull LOW (needs external pull-up). For PWM, we want clean HIGH and LOW, so push-pull is better.

### 3. Why Mode 1 vs Mode 2?
- Mode 1: HIGH when CNT < CCR (normal)
- Mode 2: HIGH when CNT > CCR (inverted)

For most applications, Mode 1 gives intuitive behavior.

### 4. Why Different ARR Values?
Higher ARR = finer duty control (e.g., ARR=1000 gives 0.1% resolution)
Lower ARR = higher max frequency (e.g., ARR=100 needed for 10kHz)

### 5. How Does the Duty Bar Work?
```c
int bars = duty_percent / 2;  // 50 chars for 100%
for (int i = 0; i < 50; i++)
{
    if (i < bars)
        printf("█");  // Filled
    else
        printf("░");  // Empty
}
```

Result: Visual representation of duty cycle!

---

## 🔍 Testing Checklist

### Base Functionality
- [ ] System boots, terminal shows UI
- [ ] Space bar toggles PWM (LED changes)
- [ ] Key 'A' sets 100Hz (verify on oscilloscope)
- [ ] Key 'E' sets 10kHz
- [ ] '+' increases duty by 5%
- [ ] '-' decreases duty by 5%
- [ ] '5' sets duty to 50%
- [ ] Duty bar updates correctly
- [ ] Clock increments every second

### Enhancement
- [ ] S1 decreases duty by 1%
- [ ] S2 increases duty by 1%
- [ ] S3 cycles to previous frequency
- [ ] S4 cycles to next frequency
- [ ] Buttons don't wrap around limits

### Oscilloscope Verification
For 1kHz, 50% duty:
- Period should be 1ms (1000μs)
- HIGH time should be 500μs
- LOW time should be 500μs

---

## 🐛 Common Issues & Solutions

### Issue 1: PWM not outputting
**Solution:**
- Check AF configuration: `_GPIO_SetPinAlternateFunction(GPIOA, 4, 4);`
- Verify timer is started: `Timer_Start(TIM14);`
- Check output enable: `Timer_EnableOutput(TIM14, TIMER_CHANNEL1);`

### Issue 2: Wrong frequency
**Solution:**
- Verify prescaler and period calculations
- Check system clock is actually 32MHz
- Use oscilloscope to measure

### Issue 3: Terminal garbled
**Solution:**
- Check baud rate: 192300
- Verify USART2 initialization
- Check PA2/PA3 connections

### Issue 4: Duty cycle not changing
**Solution:**
- Verify CCR is being updated
- Check if timer is running
- Ensure ARR is set correctly

---

## 📊 Testing the Application

### Oscilloscope Setup
1. Connect probe to PA4
2. Set timebase appropriate for frequency:
   - 100 Hz: 5ms/div
   - 1 kHz: 500μs/div
   - 10 kHz: 50μs/div

### Expected Waveforms

**1kHz, 50% Duty:**
```
     ┌──┐  ┌──┐  ┌──┐
     │  │  │  │  │  │
─────┘  └──┘  └──┘  └──
     500μs
     <────>
     1ms period
     <──────────>
```

**1kHz, 75% Duty:**
```
     ┌─────┐  ┌─────┐
     │     │  │     │
─────┘     └──┘     └──
     750μs
     <────────>
```

---

## 🚀 Quick Start Guide

1. **Connect Hardware:**
   - PA4 → Oscilloscope
   - PC6 → LED (with resistor)
   - PA2/PA3 → USB-Serial adapter

2. **Build & Flash:**
   ```bash
   # Your build commands here
   ```

3. **Open Terminal:**
   - Baud: 192300
   - Data: 8N1
   - Terminal: PuTTY/TeraTerm

4. **Test Sequence:**
   - Press SPACE → LED should light
   - Press 'C' → Set to 1kHz
   - Press '5' → Set to 50% duty
   - Verify on oscilloscope: 1kHz, 50% duty

---

## 📚 References

- STM32G031 Datasheet: Timer specifications
- Your existing libraries: clock.h, gpio.h, Timer.h, usart.h
- ANSI Escape Codes: Terminal control sequences

---

## ✅ Deliverables

1. ✅ main.c (complete application)
2. ✅ Documentation of clock math
3. ✅ Pin configuration documentation
4. ✅ Frequency table with calculations
5. ✅ Oscilloscope screenshots (recommended)

---

**Good luck with your lab! 🎉**
