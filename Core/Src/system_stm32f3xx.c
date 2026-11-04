#include "system_stm32f3xx.h"

#define HSI_VALUE 8000000UL

uint32_t SystemCoreClock = HSI_VALUE;

void SystemInit(void)
{
    /* The reset clock configuration already selects the 8 MHz HSI clock. */
    SystemCoreClock = HSI_VALUE;
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = HSI_VALUE;
}
