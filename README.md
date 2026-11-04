# STM32F3 Discovery SWO test

A small Cortex-M4 firmware project modeled on the `cortex-debug-nucleo-swo` test. It targets the STM32F303VCT6 on the STM32F3DISCOVERY board and exercises ITM stimulus ports 0 and 1 through SWO.

## Firmware output

`Core/Src/main.c` sends a pair of lines every 200 ms. The firmware runs from the reset-default 8 MHz HSI clock so that the CPU frequency in the debugger profile is known and stable.

| ITM port | Example | Encoding |
|---|---|---|
| 0 | `port0 line 0` | ASCII |
| 1 | `port1 řádek 0: žluťoučký kůň` | UTF-8 |

The eight user LEDs on PE8–PE15 show the low byte of the increasing message counter in binary. ITM configuration and stimulus-port enables are left to the debugger. Each output byte gets a bounded 1,000-read retry; if the port remains busy, the rest of that line is dropped rather than waiting indefinitely.

## Build

Install CMake, Ninja and GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`) and make them available on `PATH`. Open this directory as a CMake project in CLion and select the `Debug` preset, or build from PowerShell:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

For an optimized build, use `Release` in both commands. The build directory contains the ELF, Intel HEX, raw binary, and linker map for each preset. The project includes only the small CMSIS-compatible Cortex-M and peripheral register subset used by this test; it does not depend on STM32 HAL or the full STM32CubeF3 package.

### Host firmware tests

With a host GCC compiler on `PATH`, run the mocked register/ITM tests from PowerShell:

```powershell
New-Item -ItemType Directory -Force -Path 'build\host-tests' | Out-Null
gcc -std=c11 -Wall -Wextra -Werror -Wno-unused-function `
  -ffunction-sections -fdata-sections -Wl,--gc-sections `
  -DSTM32F303xC -ICore\Inc -IDrivers\CMSIS\Include `
  -IDrivers\CMSIS\Device\ST\STM32F3xx\Include `
  tests\test_firmware.c -o build\host-tests\test_firmware.exe
.\build\host-tests\test_firmware.exe
```

These checks cover the DBGMCU register offset, disabled tracing, transient and persistent ITM backpressure, and complete ASCII/UTF-8 lines.

## Debug with SWO

The profiles in [`.idea/debug-profiles.json`](.idea/debug-profiles.json) configure two console decoders (ITM ports 0 and 1), with an 8 MHz CPU clock and 1 MHz SWO clock:

- **STM32F3 Discovery - OpenOCD SWO** uses OpenOCD's `board/stm32f3discovery.cfg` board script.
- **STM32F3 Discovery - ST-LINK SWO** uses `ST-LINK_gdbserver.exe`.

Install Node.js, OpenOCD or STM32CubeProgrammer/ST-LINK GDB Server, and the CLion Cortex-Debug and Serial Port Monitor plugins. Ensure the debugger executables and Arm GDB/objdump are on `PATH`; edit `serverpath` in the profile if your tools are installed elsewhere. Select the built `stm32f3discovery-swo.elf` executable and one profile, then resume after the debugger stops at `main`.

### Board routing

The board's SWO signal is PB3 (alternate function 0). The firmware configures PB3 for trace and enables the MCU trace output. On STM32F3DISCOVERY, solder bridge SB10 must be closed to route PB3/SWO to the SWO signal on the CN3 debug connector; verify its population and your board revision against the schematic. For example, ST's [MB1035-F303C-E02 schematic](https://www.st.com/resource/en/schematic_pack/mb1035-f303c-e02_schematic.pdf) marks SB10 as DNF. The on-board ST-LINK can program and debug over SWD, but SWO capture depends on whether the probe/board revision actually routes that signal to the probe. If the debugger shows empty SWO tabs, verify the physical routing first.

For an external SWO receiver, connect its input to the board's SWO/PB3 signal and share ground; use a 3.3 V-compatible receiver. This project does not send SWO through the ST-LINK virtual COM port.

## Manual checks

- Confirm the eight user LEDs count in binary while the target runs.
- Confirm port 0 displays `port0 line N` as ASCII and port 1 displays the UTF-8 text with intact Czech characters; `N` should increase about five times per second.
- Pause and resume the target: output should pause and then continue with the next count.
- Disable SWO or a decoder port: the LED counter must keep running, with no firmware hang.

No hardware run is claimed here; SWO capture must be checked with the actual board revision and probe wiring.
