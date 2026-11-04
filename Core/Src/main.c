#include "stm32f3xx.h"
#include "system_stm32f3xx.h"

#include <stdint.h>

#define LED_MASK (0xFFUL << 8)
#define LED_MODE_MASK (0xFFFFUL << 16)
#define SWO_PIN (1UL << 3)
#define SWO_PERIOD_MS 200UL

#ifndef ITM_WRITE_POLL_LIMIT
#define ITM_WRITE_POLL_LIMIT 1000UL
#endif

#ifndef ITM_PORT_READY
#define ITM_PORT_READY(port) (ITM->PORT[port].u32 != 0UL)
#endif

#ifndef ITM_PORT_WRITE
#define ITM_PORT_WRITE(port, value) (ITM->PORT[port].u8 = (uint8_t)(value))
#endif

static volatile uint32_t milliseconds;

void SysTick_Handler(void)
{
    milliseconds++;
}

static void gpio_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOEEN;
    (void)RCC->AHBENR;

    GPIOE->MODER = (GPIOE->MODER & ~LED_MODE_MASK) | (0x5555UL << 16);
    GPIOE->OTYPER &= ~LED_MASK;
    GPIOE->OSPEEDR = (GPIOE->OSPEEDR & ~LED_MODE_MASK) | (0x5555UL << 16);
    GPIOE->PUPDR &= ~LED_MODE_MASK;

    GPIOB->MODER = (GPIOB->MODER & ~(3UL << 6)) | (2UL << 6);
    GPIOB->OTYPER &= ~SWO_PIN;
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~(3UL << 6)) | (3UL << 6);
    GPIOB->PUPDR &= ~(3UL << 6);
    GPIOB->AFR[0] &= ~(0xFUL << 12);
    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;
}

static void systick_init(void)
{
    SysTick->LOAD = (SystemCoreClock / 1000UL) - 1UL;
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

static void itm_write_string(uint32_t port, const char *text)
{
    if (port >= 32UL || (ITM->TCR & ITM_TCR_ITMENA_Msk) == 0UL || (ITM->TER & (1UL << port)) == 0UL) {
        return;
    }

    while (*text != '\0') {
        uint32_t polls_remaining = ITM_WRITE_POLL_LIMIT;
        while (polls_remaining != 0UL && !ITM_PORT_READY(port)) {
            polls_remaining--;
        }
        if (polls_remaining == 0UL) {
            return;
        }
        ITM_PORT_WRITE(port, (uint8_t)*text++);
    }
}

static void emit_line(uint32_t port, const char *prefix, uint32_t count, const char *suffix)
{
    char line[64];
    char digits[10];
    uint32_t length = 0UL;
    uint32_t digit_count = 0UL;

    while (*prefix != '\0') {
        line[length++] = *prefix++;
    }
    do {
        digits[digit_count++] = (char)('0' + (count % 10UL));
        count /= 10UL;
    } while (count != 0UL);
    while (digit_count != 0UL) {
        line[length++] = digits[--digit_count];
    }
    while (*suffix != '\0') {
        line[length++] = *suffix++;
    }
    line[length] = '\0';

    itm_write_string(port, line);
}

static void emit_swo_lines(uint32_t count)
{
    emit_line(0UL, "port0 line ", count, "\n");
    emit_line(1UL, "port1 řádek ", count, ": žluťoučký kůň\n");
}

#ifndef FIRMWARE_HOST_TEST
int main(void)
{
    uint32_t last_emit = 0UL;
    uint32_t count = 0UL;

    gpio_init();
    systick_init();

    for (;;) {
        const uint32_t now = milliseconds;
        if ((uint32_t)(now - last_emit) >= SWO_PERIOD_MS) {
            last_emit += SWO_PERIOD_MS;
            emit_swo_lines(count);
            GPIOE->ODR = (GPIOE->ODR & ~LED_MASK) | ((count & 0xFFUL) << 8);
            count++;
        }
        __WFI();
    }
}
#endif
