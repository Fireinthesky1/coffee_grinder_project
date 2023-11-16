#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>

//TODO(James): add to emacs config file the code for moving
//             lines up and down

/*
1) A GPIO input will tell us to
      a) start the machine
      b) tare
2) We will constantly read the data off of the chip
3) We will stop the machine when we read 15 grams
*/

/*
  NOTE:
  1) A falling edge of DOUT will trigger an interrupt
  2) This interrupt starts a .1 second timer to account for
     T1
  3) The T1 interrupt will set PD_SCK high and trigger 
     another .1 ms timer to account for T2
  4) The T2 interrupt will read DOUT and start a T3 timer
  5) The T3 interrupt will set PD_SCK low and start T4
  6) The T4 interrupt will set PD_SCK high and start T2
     T4 will also increment a global variable pulse
  7) etc until 25 periods have gone by

  NOTE:
  T1 <- .1 second timer
  T2 <- .1 second timer
  T3 <-  1 second timer
  T4 <-  1 second timer

 */

uint32_t current_weight = 0;

//TODO: Figure out the load value
// data collection timer
void t0_init(void)
{
  
}

//NOTE: .1 second timer
void t1_init(void)
{
  
}

//NOTE: .1 second timer
void t2_init(void)
{
  
}

//NOTE: 1 second timer
void t3_init(void)
{
  
}

//NOTE: 1 second timer
void t4_init(void)
{
  
}

////////////////////////////////////////////////////////////////////////////////

// collects DOUT data
void t0_isr(void)
{
  // disable timer0 interrupts
  // clear timer0 interrupts
  // read DOUT
  current_weight << 1;                                                          //TODO: check that this works
  current_weight += GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);                   //TODO: check that this works
}

// called when dout goes low
//NOTE: Time between when dout goes low and data transfer begins
void t1_isr(void)
{
  // disable timer1 interrupts
  // clear timer1 interrupts
  // enable timer2 interrupts NOTE: Data transition timer
}

//NOTE: Timer between when clock pulse is recieved to data being stable
void t2_isr(void)
{
  // disable timer2 interrupts
  // clear timer2 interrupts
  // enable timer3 interrupts NOTE: clock HI timer
  // enable timer0 interrupts NOTE: Data collect timer
}

void t3_isr(void)
{
  // disable timer1 interrupts
  // clear timer1 interrupts
  // do stuff
  // enable timer2 interrupts
}

void t4_isr(void)
{
  // disable timer1 interrupts
  // clear timer1 interrupts
  // do stuff
  // enable timer2 interrupts
}

//NOTE::5 minute timeout timer.
void t5_isr(void)
{
  // disable timer5 interrupts
  // clear timer5 interrupts
  // stop the machine
}

//NOTE::TRIGGERED WHEN DOUT GOES LOW
void portb_isr(void)
{
}

//NOTE::PB0 collects DOUT; PB1 provides PD_SCK
void gpio_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

  // DOUT [input]
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);

  // PD_SCK [output]
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);

  GPIOPadConfigSet(GPIO_PORTB_BASE,                                             //TODO: does this work for both pins?
                   GPIO_PIN_0 | GPIO_PIN_1,
                   GPIO_STRENGTH_2MA,
                   GPIO_PIN_TYPE_STD);

  //NOTE::DOUT TRIGGERS THE INTERRUPT
  GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);

  GPIOIntRegister(GPIO_PORTB_BASE, portb_isr);
}

void tare(void)
{
}

//NOTE::THIS FUNCTION STARTS THE MACHINE
void machine_start(void)
{
}

//NOTE::THIS FUNCTION STOPS THE MACHINE
void machine_stop(void)
{
}

int main(void)
{
  // TODO: FUNCTION THAT STARTS THE MACHINE
  // TODO: FIGURE OUT TARE MATH
  return 0;
}
