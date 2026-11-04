#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f3xx.h"

static ITM_Type mock_itm;

#undef ITM
#define ITM (&mock_itm)

typedef struct {
    uint32_t busy_reads;
    uint32_t ready_checks;
    uint8_t bytes[128];
    size_t byte_count;
} MockStimulusPort;

static MockStimulusPort mock_ports[32];
static uint32_t failures;

static int mock_stimulus_ready(uint32_t port)
{
    MockStimulusPort *const state = &mock_ports[port];
    state->ready_checks++;
    if (state->busy_reads != 0UL) {
        state->busy_reads--;
        return 0;
    }
    return 1;
}

static void mock_stimulus_write(uint32_t port, uint8_t value)
{
    MockStimulusPort *const state = &mock_ports[port];
    if (state->byte_count < sizeof(state->bytes)) {
        state->bytes[state->byte_count++] = value;
    }
}

#define ITM_PORT_READY(port) mock_stimulus_ready(port)
#define ITM_PORT_WRITE(port, value) mock_stimulus_write((port), (value))
#define ITM_WRITE_POLL_LIMIT 4UL
#define FIRMWARE_HOST_TEST
#include "../Core/Src/main.c"

uint32_t SystemCoreClock = 8000000UL;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        failures++;
    }
}

static void reset_test_state(void)
{
    memset(&mock_itm, 0, sizeof(mock_itm));
    memset(mock_ports, 0, sizeof(mock_ports));
    mock_itm.TCR = ITM_TCR_ITMENA_Msk;
    mock_itm.TER = (1UL << 0) | (1UL << 1);
    mock_itm.PORT[0].u32 = 1UL;
    mock_itm.PORT[1].u32 = 1UL;
}

static void check_port_bytes(uint32_t port, const char *expected, const char *description)
{
    const size_t expected_count = strlen(expected);
    check(mock_ports[port].byte_count == expected_count, description);
    if (mock_ports[port].byte_count == expected_count) {
        check(memcmp(mock_ports[port].bytes, expected, expected_count) == 0, description);
    }
}

static void test_dbgmcu_layout(void)
{
    check(offsetof(DBGMCU_TypeDef, CR) == sizeof(uint32_t),
          "DBGMCU CR follows the IDCODE register");
}

static void test_disabled_trace_does_not_poll(void)
{
    reset_test_state();
    mock_itm.TCR = 0UL;
    itm_write_string(0UL, "disabled");
    check(mock_ports[0].ready_checks == 0UL && mock_ports[0].byte_count == 0UL,
          "disabled ITM does not poll or write");

    reset_test_state();
    mock_itm.TER = 0UL;
    itm_write_string(0UL, "disabled");
    check(mock_ports[0].ready_checks == 0UL && mock_ports[0].byte_count == 0UL,
          "disabled stimulus port does not poll or write");
}

static void test_transient_busy_port_retries(void)
{
    reset_test_state();
    mock_itm.PORT[0].u32 = 0UL;
    mock_ports[0].busy_reads = 2UL;
    itm_write_string(0UL, "A");
    check(mock_ports[0].ready_checks == 3UL, "busy ITM port is retried until ready");
    check_port_bytes(0UL, "A", "retried byte is written");
}

static void test_permanently_busy_port_is_bounded(void)
{
    reset_test_state();
    mock_itm.PORT[0].u32 = 0UL;
    mock_ports[0].busy_reads = 100UL;
    itm_write_string(0UL, "X");
    check(mock_ports[0].ready_checks == ITM_WRITE_POLL_LIMIT,
          "permanently busy ITM port uses the full bounded poll budget");
    check(mock_ports[0].byte_count == 0UL, "busy ITM port does not write a byte");
}

static void test_ascii_and_utf8_lines(void)
{
    reset_test_state();
    emit_swo_lines(42UL);
    check_port_bytes(0UL, "port0 line 42\n", "port 0 emits the complete ASCII line");
    check_port_bytes(1UL, "port1 řádek 42: žluťoučký kůň\n",
                     "port 1 emits the complete UTF-8 line");
}

int main(void)
{
    test_dbgmcu_layout();
    test_disabled_trace_does_not_poll();
    test_transient_busy_port_retries();
    test_permanently_busy_port_is_bounded();
    test_ascii_and_utf8_lines();

    if (failures != 0UL) {
        fprintf(stderr, "%lu test(s) failed\n", (unsigned long)failures);
        return 1;
    }

    puts("All firmware host tests passed.");
    return 0;
}