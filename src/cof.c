#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>
#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/timer.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>

#define data_pin  GPIO_PIN_0
#define clock_pin GPIO_PIN_1

//TODO: WHY AREN'T WE GETTING TIMER INTERRUPTS?

/*
1) At startup we run one conversation to tare and then we constantly
2) We will constantly read the data off of the chip
3) We will stop the machine when we read 15 grams

  NOTE:
  T1 <- 10 samples per second
  T2 <- 80 samples per second

  PB0 dout
  PB1 clock
  PF1 machine_on
*/

volatile uint32_t current_value  = 0;
volatile uint32_t offset_value   = 0; // tare

volatile bool     machine_on     = true;
volatile bool     chip_ready     = false;
volatile bool     data_ready     = false;
volatile bool     data_rate_fast = false;

uint32_t read(void)
{
  // start conversation
  GPIOPinWrite(GPIO_PORTB_BASE, clock_pin, clock_pin);
  SysCtlDelay(80); // <= 1 microsecond
  GPIOPinWrite(GPIO_PORTB_BASE, clock_pin, 0x0);

  // wait for dout to go low
  while(GPIOPinRead(GPIO_PORTB_BASE, data_pin) == 0x1)
    {
    }

  int i;
  int temp = 0;

  for(i = 0; i < 24; i++)
    {
      GPIOPinWrite(GPIO_PORTB_BASE, clock_pin, clock_pin);
      SysCtlDelay(0x1B); // 27*3=81 <= 1 microsecond
      temp = (temp << 1);
      if(GPIOPinRead(GPIO_PORTB_BASE, data_pin))
        {
          temp++;
        }
      GPIOPinWrite(GPIO_PORTB_BASE, clock_pin, 0x0);
    }

  return temp;
}

//NOTE: slow timer 10 sps
void t1_isr(void)
{

  TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  current_value = read() - offset_value;

}

//NOTE: fast timer 80 sps
void t2_isr(void)
{
  TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

  current_value = read() - offset_value;
}

//NOTE: slow timer (10 sps)
void t1_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
  {
  }

  TimerDisable(TIMER1_BASE, TIMER_BOTH);

  TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);

  TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

  TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet() / 10)); // 10 sps

  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER2_BASE, TIMER_A, t1_isr);
}

// NOTE: Fast Timer (80 sps)
void t2_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
    {
    }

  TimerDisable(TIMER2_BASE, TIMER_BOTH);

  TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_SYSTEM);

  TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);

  TimerLoadSet(TIMER2_BASE, TIMER_A, (SysCtlClockGet() / 80)); // 80 sps

  TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER2_BASE, TIMER_A, t2_isr);
}

//NOTE::PB0 collects DOUT; PB1 provides PD_SCK
//NOTE::PF3 provides machine_on / machine_off
void gpio_init(void)
{

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
  {}

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
  {
  }

  // PF3 [MACHINE ON] (green means on)
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

  // DOUT [input]
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);

  // PB1 PD_SCK [output]
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);

  GPIOPadConfigSet(GPIO_PORTB_BASE,
                   GPIO_PIN_0 | GPIO_PIN_1,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

  GPIOPadConfigSet(GPIO_PORTF_BASE,
                   GPIO_PIN_3,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

}

void tare(void)
{
  offset_value = read();
}

void main(void)
{
  // SET SYSTEM CLOCK TO 80MHz
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 |
                 SYSCTL_USE_PLL |
                 SYSCTL_OSC_MAIN |
                 SYSCTL_XTAL_16MHZ);

  // INITIALIZE
  IntMasterEnable();
  gpio_init();
  t1_init();
  t2_init();

  tare();

  // start the machine
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, machine_on << 3);

  // SET TIMER BASED ON DATA RATE
  if(data_rate_fast)
    {
      TimerEnable(TIMER2_BASE, TIMER_BOTH);
    }
  else
    {
      TimerEnable(TIMER1_BASE, TIMER_BOTH);
    }


  while(1)
    {
      //NOTE: need data to gram conversion here
      //if(current_value / digital_max * 1000 > 15)
      //  {
      //    machine_on = false;
      //  }

      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, machine_on << 3);
    }
}
