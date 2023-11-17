# coffee_grinder_project
Using the TM4C123GH6PM microcontroller with a 1 kg load cell send a signal to turn on a coffee grinder and then turn it off when we sense 15 grams off coffee on the load cell.

## Details
        1) On startup a machine_on signal is set high on GPIO PB3
        2) GPIO Interrupts occur on the falling edge of the DOUT signal
        3) The portb_isr starts a T1 timer to account for the time between
           dout going low on the HX711 and the time that the chip is ready
           to receive clock signals. Additionally the portb_isr disables portb
           interrupts
        4) The t1_isr starts t2 and disables t1 interrupts.
        5) t2 interrupts occur every 1 microsecond. The t2_isr toggles the clock
           signal (PB1) and (on falling edges) reads the current dout bit into a
           variable called current value. Additionally it increments a variable
           called num_pulses.
        6) When we reach 25 pulses the conversation is over and we disable T2
           interrupts, Reenable Port B interrupts and stores the current weight.

## TODO
        1) Figure out the math to get the current weight in grams
        2) Figure out how we start and end a conversation.
        3) Figure out data collection rate.
        4) Bullet proof timer load values
        5) Verify that we're getting 80MHz clock
## Tools to look into
	1) OpenOCD
	2) CMAKE (to use arm-none-eabi-gcc)
	3) CMAKE (toolchain files)
## OpenOCD
	1) openocd -f board/ek-tm4c123gxl.cfg -c init -c "program firmware.bin; reset" -c shutdown
## arm-none-eabi-gcc
	1) arm-none-eabi-gcc -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb
	2) (for enabling the fpu) -mfpu=fpv4-sp-d16 -mfloat-abi=hard
