#ifndef STM32F303XC_H
#define STM32F303XC_H

#include <stdint.h>
#include "core_cm4.h"

#define STM32F303xC 1

typedef struct {
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t APB2RSTR;
    __IO uint32_t APB1RSTR;
    __IO uint32_t AHBENR;
} RCC_TypeDef;

typedef struct {
    __IO uint32_t IDCODE;
    __IO uint32_t CR;
} DBGMCU_TypeDef;

#define GPIOB ((GPIO_TypeDef *)0x48000400UL)
#define GPIOE ((GPIO_TypeDef *)0x48001000UL)
#define RCC ((RCC_TypeDef *)0x40021000UL)
#define DBGMCU ((DBGMCU_TypeDef *)0xE0042000UL)

#define RCC_AHBENR_GPIOBEN (1UL << 18)
#define RCC_AHBENR_GPIOEEN (1UL << 21)
#define DBGMCU_CR_TRACE_IOEN (1UL << 5)

#endif
