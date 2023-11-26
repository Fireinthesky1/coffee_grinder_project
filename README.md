# coffee_grinder_project
Using the TM4C123GH6PM microcontroller with a 1 kg load cell send a signal to turn on a coffee grinder and then turn it off when we sense 15 grams of coffee on the load cell.

## Details
        1) On startup a machine_on signal is set high on GPIO PF3 (GREEN LED)
        2) 10 times a second we initiate a conversation with the chip and read
           out data.
        3) In the main loop we constantly check (using an adc to gram eqation)
           if the load cell has sensed 15 grams of coffee.
        4) If so we record the final weight (for testing pursposes), turn off the
           LED and enter an empty infinite loop

## TODO
        1) Fine Tune the adc to gram math.
        2) Figure out how to set fast data collection rate.
## NOTE
        1) We're very close but we are getting readings that are off by +- 2 grams
        2) We need to collect more calibration data to furthur fine tune the 
           adc-gram equation
