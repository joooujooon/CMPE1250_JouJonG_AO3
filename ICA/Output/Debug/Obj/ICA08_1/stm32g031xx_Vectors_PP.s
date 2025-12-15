# 0 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
# 61 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
        .syntax unified
# 73 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
.macro VECTOR Name=
        .section .vectors, "ax"
        .code 16
        .word \Name
.endm




.macro EXC_HANDLER Name=



        .section .vectors, "ax"
        .word \Name



        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b
.endm




.macro ISR_HANDLER Name=



        .section .vectors, "ax"
        .word \Name
# 116 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b

.endm




.macro ISR_RESERVED
        .section .vectors, "ax"
        .word 0
.endm




.macro ISR_RESERVED_DUMMY
        .section .vectors, "ax"
        .word Dummy_Handler
.endm







        .extern __stack_end__
        .extern Reset_Handler
        .extern HardFault_Handler
# 163 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
        .section .vectors, "ax"
        .code 16
        .balign 256
        .global _vectors
_vectors:



        VECTOR __stack_end__
        VECTOR Reset_Handler
        EXC_HANDLER NMI_Handler
        VECTOR HardFault_Handler

        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED





        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        EXC_HANDLER SVC_Handler

        ISR_RESERVED



        ISR_RESERVED
        EXC_HANDLER PendSV_Handler
        EXC_HANDLER SysTick_Handler




        ISR_HANDLER WWDG_IRQHandler
        ISR_HANDLER PVD_IRQHandler
        ISR_HANDLER RTC_TAMP_IRQHandler
        ISR_HANDLER FLASH_IRQHandler
        ISR_HANDLER RCC_IRQHandler
        ISR_HANDLER EXTI0_1_IRQHandler
        ISR_HANDLER EXTI2_3_IRQHandler
        ISR_HANDLER EXTI4_15_IRQHandler
        ISR_RESERVED
        ISR_HANDLER DMA1_Channel1_IRQHandler
        ISR_HANDLER DMA1_Channel2_3_IRQHandler
        ISR_HANDLER DMA1_Ch4_5_DMAMUX1_OVR_IRQHandler
        ISR_HANDLER ADC1_IRQHandler
        ISR_HANDLER TIM1_BRK_UP_TRG_COM_IRQHandler
        ISR_HANDLER TIM1_CC_IRQHandler
        ISR_HANDLER TIM2_IRQHandler
        ISR_HANDLER TIM3_IRQHandler
        ISR_HANDLER LPTIM1_IRQHandler
        ISR_HANDLER LPTIM2_IRQHandler
        ISR_HANDLER TIM14_IRQHandler
        ISR_RESERVED
        ISR_HANDLER TIM16_IRQHandler
        ISR_HANDLER TIM17_IRQHandler
        ISR_HANDLER I2C1_IRQHandler
        ISR_HANDLER I2C2_IRQHandler
        ISR_HANDLER SPI1_IRQHandler
        ISR_HANDLER SPI2_IRQHandler
        ISR_HANDLER USART1_IRQHandler
        ISR_HANDLER USART2_IRQHandler
        ISR_HANDLER LPUART1_IRQHandler


        .section .vectors, "ax"
_vectors_end:
# 255 "C:\\Users\\Jou Jon\\Documents\\GitHub\\Embedded_CMPE1250_JouJonG\\ICA\\STM32G0xx\\Source\\stm32g031xx_Vectors.s"
        .section .init.Dummy_Handler, "ax"
        .thumb_func
        .weak Dummy_Handler
        .balign 2
Dummy_Handler:
        1: b 1b
