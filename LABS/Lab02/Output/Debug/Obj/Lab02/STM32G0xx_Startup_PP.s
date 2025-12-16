# 0 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
# 84 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        .syntax unified
# 108 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        .global reset_handler
        .global Reset_Handler
        .equ reset_handler, Reset_Handler
        .section .init.Reset_Handler, "ax"
        .balign 2
        .thumb_func
Reset_Handler:
# 128 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        bl SystemInit
# 184 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        bl _start





        .weak SystemInit





        .section .init_array, "aw"
        .balign 4
        .word SystemCoreClockUpdate
# 221 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        .weak HardFault_Handler
        .section .init.HardFault_Handler, "ax"
        .balign 2
        .thumb_func
HardFault_Handler:



        ldr R1, =0xE000ED2C
        ldr R2, [R1]
        cmp R2, #0

.LHardFault_Handler_hfLoop:
        bmi .LHardFault_Handler_hfLoop




        movs R0, #4
        mov R1, LR
        tst R0, R1
        bne .LHardFault_Handler_Uses_PSP
        mrs R0, MSP
        b .LHardFault_Handler_Pass_StackPtr
.LHardFault_Handler_Uses_PSP:
        mrs R0, PSP
.LHardFault_Handler_Pass_StackPtr:
# 258 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\LABS\\Lab02\\STM32G0xx\\Source\\STM32G0xx_Startup.s"
        movs R3, #1
        lsls R3, R3, #31
        orrs R2, R3
        str R2, [R1]







        ldr R1, [R0, #24]
        adds R1, #2
        str R1, [R0, #24]

        bx LR
