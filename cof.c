#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
//#include <inc/tm4c123gh6pm.h>
#include <driverlib/interrupt.h>

//TODO: CONVERSATION TO END WHEN EXACTLY 25 PULSES HAVE BEEN SENT

/*
1) A GPIO input will tell us to
      a) start the machine
      b) tare
2) We will constantly read the data off of the chip
3) We will stop the machine when we read 15 grams
*/

/*
  NOTE:
  1) two timers first timer starts when dout goes low
  2) second timer is 50% duty cycle pwm, reads dout on falling edge

  NOTE:
  T1 <- .1 microsecond timer
  T2 <-  1 microsecond timer
 */

uint32_t current_weight = 0;
uint32_t counter        = 0;
uint32_t divisions      = 256;
uint32_t set            = 50;
uint32_t num_pulses     = 0;
bool     machine_on     = true;
// PB0 dout
// PB1 clock
// PB2 machine_on

//NOTE: .1 microsecond timer
void t1_isr(void)
{
  TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

  TimerEnable(TIMER2_BASE, TIMER_BOTH);
}

//NOTE: PWM timer 50 % duty cycle
//NOTE: reads dout on falling edge
//NOTE: PB1 is sck
void t2_isr(void)
{
  // toggle the clk
  GPIOPinWrite(GPIO_PORTB_BASE,
               GPIO_PIN_1,
               ~GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1));
  // are we on a falling edge?
  if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) == 0)
    {
      current_weight << 1;
      current_weight += GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
      num_pulses++;
    }
  if(num_pulses == 25)
    {
      num_pulses = 0;
      TimerIntDisable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    }
}

//NOTE::TRIGGERED WHEN DOUT GOES LOW
void portb_isr(void)
{
  GPIOIntDisable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);

  GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_0);

  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
}

////////////////////////////////////////////////////////////////////////////////

//NOTE: .1 microseconds
//NOTE: T1 timer, waits from the time that DOUT goes low till first clk
//NOTE: 8 cycles gives us .1 microseconds
//NOTE: so we will use 16 cycles and use .2 microseconds
void t1_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)){}

  TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);

  TimerDisable(TIMER1_BASE, TIMER_BOTH);

  TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

  TimerLoadSet(TIMER1_BASE, TIMER_A, 0x10); // 16 cycles .2 microseconds        // TODO: this is twice the minimum

  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER2_BASE, TIMER_A, t1_isr);
}

//NOTE: 1 microsecond timer
//NOTE: reads dout on the falling edge
//NOTE: 50% duty cycle pwm
void t2_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)){}

  TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_SYSTEM);

  TimerDisable(TIMER2_BASE, TIMER_BOTH);

  TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);

  TimerLoadSet(TIMER2_BASE, TIMER_A, 0x50); // 80 cycles 1 microsecond          // TODO: this is the minimum time

  TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER2_BASE, TIMER_A, t2_isr);
}

//NOTE::PB0 collects DOUT; PB1 provides PD_SCK
void gpio_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

  // DOUT [input]
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);

  // PB1 PD_SCK [output], PB2 machine_on [output]
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2);

  GPIOPadConfigSet(GPIO_PORTB_BASE,
                   GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

  //NOTE::DOUT TRIGGERS THE INTERRUPT
  GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);

  GPIOIntRegister(GPIO_PORTB_BASE, portb_isr);
}

void tare(void)
{
  
}

int main(void)
{
  // SET SYSTEM CLOCK TO 80MHz //TODO: MAKE SURE THIS IS ACTUALLY 80MHz
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 |
                 SYSCTL_USE_PLL |
                 SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_16MHZ);
  // start the machine
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, machine_on);
  // TODO: FIGURE OUT TARE MATH
  tare();
  while(1)
    {
      if(current_weight >= 15) // TODO: NEEDS TO BE 15 GRAMS
        {
          //TODO: could we just return an shut off the machine here?
          machine_on = false;
          GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, machine_on);
        }
    }
  return 0;
}
