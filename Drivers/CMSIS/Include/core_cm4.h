#ifndef CORE_CM4_H
#define CORE_CM4_H

#include <stdint.h>

#define __IO volatile

typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __IO uint32_t CALIB;
} SysTick_Type;

#define SysTick ((SysTick_Type *)0xE000E010UL)
#define SysTick_CTRL_ENABLE_Msk (1UL << 0)
#define SysTick_CTRL_TICKINT_Msk (1UL << 1)
#define SysTick_CTRL_CLKSOURCE_Msk (1UL << 2)

typedef union {
    __IO uint8_t u8;
    __IO uint16_t u16;
    __IO uint32_t u32;
} ITM_Port32_Type;

typedef struct {
    ITM_Port32_Type PORT[32];
    uint32_t RESERVED0[864];
    __IO uint32_t TER;
    uint32_t RESERVED1[15];
    __IO uint32_t TPR;
    uint32_t RESERVED2[15];
    __IO uint32_t TCR;
} ITM_Type;

#define ITM ((ITM_Type *)0xE0000000UL)
#define ITM_TCR_ITMENA_Msk (1UL << 0)

static inline void __WFI(void)
{
    __asm volatile ("wfi" ::: "memory");
}

#endif
