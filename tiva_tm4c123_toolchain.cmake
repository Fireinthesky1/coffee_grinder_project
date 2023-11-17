#TOOLCHAIN FILE

# specify the c compiler
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)

#specify the asm compiler
set(CMAKE_ASM_COMPILER /usr/bin/arm-none-eabi-as)

# specify the linker
set(CMAKE_LINKER /usr/bin/arm-none-eabi-ld)

# specify the archiver
set(CMAKE_AR /usr/bin/arm-none-eabi-ar)

#set flags for the c compiler
set(CMAKE_C_FLAGS "--specs=nosys.specs -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb")

#set flags for the asm compiler
set(CMAKE_ASM_FLAGS "-mthumb")

#set flags for the linker
set(LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/coffee.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
