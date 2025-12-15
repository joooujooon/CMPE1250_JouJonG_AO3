# LAB 2 PWM GENERATOR - SYSTEM ARCHITECTURE

## System Block Diagram

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                         STM32G031 MICROCONTROLLER                            │
│                                                                              │
│  ┌────────────┐         ┌──────────────┐         ┌──────────────┐           │
│  │            │         │              │         │              │           │
│  │  HSI16     │────────▶│   PLL        │────────▶│  SYSCLK      │           │
│  │ 16 MHz     │         │  N=8, R=4    │         │  32 MHz      │           │
│  │            │         │              │         │              │           │
│  └────────────┘         └──────────────┘         └──────┬───────┘           │
│                                                          │                   │
│                                                          │                   │
│                         ┌────────────────────────────────┼──────────┐        │
│                         │                                │          │        │
│                         │        AHB Bus                 │          │        │
│                         │                                │          │        │
│         ┌───────────────┼────────────────────────────────┼──────────┼─────┐  │
│         │               │                                │          │     │  │
│    ┌────▼─────┐    ┌────▼─────┐                    ┌────▼─────┐    │     │  │
│    │  GPIOA   │    │  GPIOC   │                    │  TIM14   │    │     │  │
│    │          │    │          │                    │          │    │     │  │
│    │  PA4─────┼────┼──────────┼────────────────────┼─ CH1─────┼────┼─────┼─▶│
│    │  (AF4)   │    │          │    PWM Control     │  (PWM)   │    │     │  │
│    │          │    │          │                    │          │    │     │  │
│    │  PA2 TX──┼────┼──────────┼────────────┐       │  PSC ────┼────┘     │  │
│    │  PA3 RX──┼────┼──────────┼───┐        │       │  ARR     │          │  │
│    │          │    │          │   │        │       │  CCR1    │          │  │
│    │  PA0 S1──┼────┼──────────┼───┼────┐   │       │          │          │  │
│    │  PA1 S2──┼────┼──────────┼───┼──┐ │   │       └──────────┘          │  │
│    │  PA11 S3─┼────┼──────────┼───┼──┼─┼───┤                             │  │
│    │  PA12 S4─┼────┼──────────┼───┼──┼─┼───┤       ┌──────────┐          │  │
│    │          │    │          │   │  │ │   │       │  TIM16   │          │  │
│    └──────────┘    │  PC6 LED─┼───┼──┼─┼───┼───────┼─ (Clock) │          │  │
│                    │          │   │  │ │   │       │          │          │  │
│                    └──────────┘   │  │ │   │       │  PSC     │          │  │
│                                   │  │ │   │       │  ARR     │          │  │
│                    ┌──────────┐   │  │ │   │       └──────────┘          │  │
│                    │ USART2   │   │  │ │   │                             │  │
│                    │          │   │  │ │   │                             │  │
│                    │  TX ◀────┼───┘  │ │   │                             │  │
│                    │  RX ◀────┼──────┘ │   │                             │  │
│                    │          │        │   │                             │  │
│                    └──────────┘        │   │                             │  │
│                                        │   │                             │  │
│         ┌──────────────────────────────┼───┼─────────────────────────┐   │  │
│         │      Main Application        │   │                         │   │  │
│         │                               │   │                         │   │  │
│         │  ┌────────────────────────────▼───▼──────────────────┐     │   │  │
│         │  │  User Input Processing                            │     │   │  │
│         │  │  - Keyboard (USART2 RX)                           │     │   │  │
│         │  │  - Buttons (PA0, PA1, PA11, PA12)                 │     │   │  │
│         │  └────────────────┬──────────────────────────────────┘     │   │  │
│         │                   │                                         │   │  │
│         │  ┌────────────────▼──────────────────────────────────┐     │   │  │
│         │  │  State Management                                 │     │   │  │
│         │  │  - freq_index (0-4)                               │     │   │  │
│         │  │  - duty_percent (0-100)                           │     │   │  │
│         │  │  - pwm_enabled (0/1)                              │     │   │  │
│         │  │  - uptime_seconds                                 │     │   │  │
│         │  └────────────────┬──────────────────────────────────┘     │   │  │
│         │                   │                                         │   │  │
│         │  ┌────────────────▼──────────────────────────────────┐     │   │  │
│         │  │  PWM Control                                      │     │   │  │
│         │  │  - Configure timer (PSC, ARR)                     │     │   │  │
│         │  │  - Set duty cycle (CCR1)                          │     │   │  │
│         │  │  - Start/stop timer                               │     │   │  │
│         │  └────────────────┬──────────────────────────────────┘     │   │  │
│         │                   │                                         │   │  │
│         │  ┌────────────────▼──────────────────────────────────┐     │   │  │
│         │  │  UI Update                                        │     │   │  │
│         │  │  - Refresh terminal display                       │     │   │  │
│         │  │  - Update clock                                   │     │   │  │
│         │  │  - Control LED                                    │     │   │  │
│         │  └───────────────────────────────────────────────────┘     │   │  │
│         │                                                             │   │  │
│         └─────────────────────────────────────────────────────────────┘   │  │
│                                                                            │  │
└────────────────────────────────────────────────────────────────────────────┼──┘
                                                                             │
                        External Connections:                               │
                        ────────────────────                                │
                        PA4  → Oscilloscope/Load                        ◀───┘
                        PC6  → LED + Resistor
                        PA2  → USB-Serial TX
                        PA3  → USB-Serial RX
                        PA0  → Button S1 (with pull-up)
                        PA1  → Button S2 (with pull-up)
                        PA11 → Button S3 (with pull-up)
                        PA12 → Button S4 (with pull-up)
```

## Data Flow Diagram

```
User Input (Keyboard/Buttons)
            │
            ▼
    ┌───────────────┐
    │ Input Handler │
    └───────┬───────┘
            │
            ▼
    ┌───────────────┐         ┌──────────────┐
    │ State Manager │────────▶│   PWM Config │
    └───────┬───────┘         └──────┬───────┘
            │                        │
            │                        ▼
            │                 TIM14 Hardware
            │                 - PSC, ARR, CCR1
            │                        │
            ▼                        │
    ┌───────────────┐               │
    │  UI Renderer  │               │
    └───────────────┘               │
            │                        │
            ▼                        ▼
      Terminal Display         PWM Output (PA4)
```

## State Machine

```
┌─────────────────────────────────────────────────────────────────────┐
│                          APPLICATION STATE                          │
└─────────────────────────────────────────────────────────────────────┘

Initial State:
    freq_index = 0 (100 Hz)
    duty_percent = 50%
    pwm_enabled = false
    
              ┌────────────┐
              │  PWM IDLE  │
              │ (Stopped)  │
              └──────┬─────┘
                     │
                     │ User presses SPACE
                     │
                     ▼
              ┌────────────┐
              │PWM ACTIVE  │◀────┐
              │ (Running)  │     │
              └──────┬─────┘     │
                     │           │
                     │           │ Frequency/Duty Change
                     │           │ (Reconfigure & Continue)
                     │           │
                     │           │
                     ▼           │
              ┌────────────┐    │
              │ PWM UPDATE │────┘
              │(Reconfig)  │
              └────────────┘
                     │
                     │ User presses SPACE
                     │
                     ▼
              ┌────────────┐
              │  PWM IDLE  │
              │ (Stopped)  │
              └────────────┘
```

## Timer Configuration Flow

```
Start
  │
  ▼
Calculate PSC and ARR
from frequency table
  │
  ▼
Stop timer
(Clear CEN bit)
  │
  ▼
Set PSC register
PSC ← (prescaler - 1)
  │
  ▼
Set ARR register
ARR ← (period - 1)
  │
  ▼
Generate update event
(Set UG bit)
  │
  ▼
Configure PWM mode
CCMR1 ← PWM Mode 1
  │
  ▼
Calculate CCR1 value
CCR1 ← (ARR × duty%) / 100
  │
  ▼
Enable output
(Set CC1E bit)
  │
  ▼
Is PWM enabled?
  │
  ├─ Yes ─▶ Start timer (Set CEN)
  │
  └─ No ──▶ Keep stopped
  │
  ▼
Done
```

## Duty Cycle Calculation Flow

```
User Input: Duty = X%
        │
        ▼
    Validate
  (Clamp 0-100%)
        │
        ▼
    duty_percent ← X
        │
        ▼
  Get current ARR
        │
        ▼
  Calculate CCR
  CCR ← (ARR × X) / 100
        │
        ▼
  Write to CCR1 register
  TIM14->CCR1 ← CCR
        │
        ▼
  Update UI Display
        │
        ▼
     Done

Example:
  ARR = 1000
  Duty = 75%
  
  CCR = (1000 × 75) / 100
      = 75000 / 100
      = 750
      
  TIM14->CCR1 = 750
```

## PWM Waveform Generation

```
Timer Counter (CNT) Operation:
─────────────────────────────

CNT: 0 ──▶ ARR ──▶ 0 ──▶ ARR ──▶ ...
     │            │            │
     │            │            │
     ▼            ▼            ▼
   Reload       Reload       Reload

Compare Operation (PWM Mode 1):
───────────────────────────────

When CNT < CCR1:  Output = HIGH
When CNT ≥ CCR1:  Output = LOW

Example: ARR = 1000, CCR1 = 750 (75% duty)

CNT Value:  0 ───────────────▶ 750 ─────▶ 1000
            │                  │          │
Output:     HIGH               │          LOW
                               │
                         Compare Match
                               
Waveform:
            ┌──────────────────┐
            │                  │
    ────────┘                  └────────
            <──────────────────>
                  75%
            <──────────────────────────>
                     100%
```

## Interrupt-Free Operation

```
Main Loop:
┌─────────────────────────────────────┐
│                                     │
│  while(1) {                         │
│    ┌─────────────────────────────┐ │
│    │ Check USART RX (non-block)  │ │
│    └─────────┬───────────────────┘ │
│              │                      │
│              ▼                      │
│    ┌─────────────────────────────┐ │
│    │ Process if char available   │ │
│    └─────────┬───────────────────┘ │
│              │                      │
│              ▼                      │
│    ┌─────────────────────────────┐ │
│    │ Check buttons (polling)     │ │
│    └─────────┬───────────────────┘ │
│              │                      │
│              ▼                      │
│    ┌─────────────────────────────┐ │
│    │ Check TIM16 flag (1 sec)    │ │
│    └─────────┬───────────────────┘ │
│              │                      │
│              ▼                      │
│    ┌─────────────────────────────┐ │
│    │ Update clock if flag set    │ │
│    └─────────────────────────────┘ │
│                                     │
│  }                                  │
└─────────────────────────────────────┘

No interrupts required!
All operations are polled.
```

## Memory Layout

```
Flash Memory (Program):
┌──────────────────────────┐ 0x0800 0000
│  Interrupt Vector Table  │
├──────────────────────────┤
│  main.c code             │
├──────────────────────────┤
│  clock.c library         │
├──────────────────────────┤
│  gpio.c library          │
├──────────────────────────┤
│  Timer.c library         │
├──────────────────────────┤
│  usart.c library         │
├──────────────────────────┤
│  Constant data           │
│  (frequency table, etc)  │
└──────────────────────────┘

SRAM (Variables):
┌──────────────────────────┐ 0x2000 0000
│  Stack                   │
├──────────────────────────┤
│  Global variables:       │
│  - g_state (AppState)    │
│  - buttons[4]            │
└──────────────────────────┘

Peripherals:
┌──────────────────────────┐ 0x4000 0000
│  TIM14                   │ 0x4000 2000
├──────────────────────────┤
│  TIM16                   │ 0x4001 4400
├──────────────────────────┤
│  USART2                  │ 0x4000 4400
├──────────────────────────┤
│  GPIOA                   │ 0x5000 0000
├──────────────────────────┤
│  GPIOC                   │ 0x5000 0800
├──────────────────────────┤
│  RCC                     │ 0x4002 1000
└──────────────────────────┘
```

## Timing Analysis

```
Operation Timing at 32 MHz:
───────────────────────────

1. USART Character Reception:
   Baud: 192300
   Time per byte: ~52 μs
   
2. PWM Update (duty change):
   Register write: ~4 CPU cycles = 125 ns
   Effective change: Next PWM period
   
3. Timer Reconfiguration:
   ~20 CPU cycles = 625 ns
   
4. Button Debounce Check:
   ~100 cycles = 3.125 μs
   Called every 10000 iterations ≈ 1 ms
   
5. Clock Display Update:
   sprintf + USART transmit
   ~16 characters × 52 μs = ~832 μs
   Called once per second
   
6. UI Refresh:
   Full status section
   ~200 characters × 52 μs = ~10.4 ms
   Called only on state change
```

## Power Consumption Estimate

```
Component Power (Typical @ 32 MHz, 3.3V):
────────────────────────────────────────

Core (32 MHz):           ~6 mA
GPIO outputs:            ~2 mA
TIM14 (active):          ~0.5 mA
TIM16 (active):          ~0.5 mA
USART2 (active):         ~1 mA
LED (if on, 20 mA):      ~20 mA

Total (PWM active):      ~30 mA
Total (PWM idle):        ~10 mA

Power:
  3.3V × 30 mA = 99 mW (active)
  3.3V × 10 mA = 33 mW (idle)
```

## Pin Configuration Summary

```
Pin  │ Mode │ AF  │ Function      │ Direction │ Config
─────┼──────┼─────┼───────────────┼───────────┼─────────────
PA0  │ IN   │ N/A │ Button S1     │ Input     │ Pull-up
PA1  │ IN   │ N/A │ Button S2     │ Input     │ Pull-up
PA2  │ AF   │ AF1 │ USART2 TX     │ Output    │ Push-pull
PA3  │ AF   │ AF1 │ USART2 RX     │ Input     │ Pull-up
PA4  │ AF   │ AF4 │ TIM14_CH1 PWM │ Output    │ Push-pull
PA11 │ IN   │ N/A │ Button S3     │ Input     │ Pull-up
PA12 │ IN   │ N/A │ Button S4     │ Input     │ Pull-up
PC6  │ OUT  │ N/A │ Status LED    │ Output    │ Push-pull
```

This architecture provides a complete, interrupt-free, polled system 
that efficiently manages PWM generation with user control.
