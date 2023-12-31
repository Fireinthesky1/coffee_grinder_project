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
#define target    15

//TODO: account for underflow when subtracting offset value from current value
//TODO: Figure out how to enable RATE=1 (80 sps)

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
      SysCtlDelay(0x6); // 6*3 = 18 (almost 16) <= 1 microsecond
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

  current_value = read();

  int a = 1; //TEST CODE DELETE
  a += 1;  //TEST CODE DELETE
}

//NOTE: fast timer 80 sps
void t2_isr(void)
{
  TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

  current_value = read();
  int a = 1; //TEST CODE DELETE
  a += 1; //TEST CODE DELETE
}

//NOTE: slow timer (10 sps)
void t1_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
  {
  }

  TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_PIOSC);

  TimerDisable(TIMER1_BASE, TIMER_BOTH);

  TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

  TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet() / 10)); // 10 sps

  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

  TimerIntRegister(TIMER1_BASE, TIMER_A, t1_isr);
}

// NOTE: Fast Timer (80 sps)
void t2_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
    {
    }

  TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_PIOSC);

  TimerDisable(TIMER2_BASE, TIMER_BOTH);

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
  // SET SYSTEM CLOCK TO 16 MHz
  SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT);

  // INITIALIZE
  IntMasterEnable();
  gpio_init();
  t1_init();
  t2_init();

  int i;
  for(i = 0; i < 50; i++)
  {
      tare();
  }

  // THIS IS FOR TESTING
  int final_weight = 0;

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
      //NOTE: (val - 254777) / 2225
      int weight = 2 * ((int)current_value - (int)offset_value) / 2225;
      if(weight <= 0) {continue;}
      if(weight >= target)
        {
          machine_on = false;
          final_weight = weight;
          break;
        }


    }
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00);
  while(1)
  {
  }
}
