set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc REQUIRED)
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "GNU Arm assembler")
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy REQUIRED)
find_program(CMAKE_SIZE NAMES arm-none-eabi-size REQUIRED)

set(TARGET_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=soft")
set(CMAKE_C_FLAGS_INIT "${TARGET_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS_INIT "${TARGET_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${TARGET_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .elf)
