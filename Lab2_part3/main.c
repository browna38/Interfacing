// GPIO.c
// Runs on TM4C1294
// Initialize four GPIO pins as outputs.  Continually generate output to
// drive simulated stepper motor.
// Daniel Valvano
// April 3, 2014

/* This example accompanies the books
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   Volume 1 Program 4.5

   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Volume 2 Example 2.2, Program 2.8, Figure 2.32

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// PN3 is an output to positive logic LED3
// PN2 is an output to positive logic LED2
// PN1 is an output to positive logic LED1
// PN0 is an output to positive logic LED0
#include <stdint.h>
#include "PLL.h"
#include "PWM.h"


#define SPEED                    (*((volatile uint32_t *)0x4006403C))
#define GPIO_PORTN_DIR_R        (*((volatile uint32_t *)0x40064400))
#define GPIO_PORTN_AFSEL_R      (*((volatile uint32_t *)0x40064420))
#define GPIO_PORTN_DEN_R        (*((volatile uint32_t *)0x4006451C))
#define GPIO_PORTN_AMSEL_R      (*((volatile uint32_t *)0x40064528))
#define GPIO_PORTN_PCTL_R       (*((volatile uint32_t *)0x4006452C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R12     0x00001000  // GPIO Port N Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R12       0x00001000  // GPIO Port N Peripheral Ready


#define GPIO_PORTA_DATA_R       (*((volatile uint32_t *)0x400583FC))
#define GPIO_PORTA_DIR_R        (*((volatile uint32_t *)0x40058400))
#define GPIO_PORTA_AFSEL_R      (*((volatile uint32_t *)0x40058420))
#define GPIO_PORTA_DEN_R        (*((volatile uint32_t *)0x4005851C))
#define GPIO_PORTA_AMSEL_R      (*((volatile uint32_t *)0x40058528))
#define PA5                     (*((volatile uint32_t *)0x40058080))
#define GPIO_PORTA_PCTL_R       (*((volatile uint32_t *)0x4005852C))
#define PA5_SW_PRESSED          0x20  // value read from location PA5 when external switch is pressed
#define PA5_SW_NO_PRESSED       0x00  // value read from location PA5 when external switch is released
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R0      0x00000001  // GPIO Port A Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R0        0x00000001  // GPIO Port A Peripheral Ready


#define GPIO_PORTJ_DATA_R       (*((volatile uint32_t *)0x400603FC))
#define GPIO_PORTJ_DIR_R        (*((volatile uint32_t *)0x40060400))
#define GPIO_PORTJ_AFSEL_R      (*((volatile uint32_t *)0x40060420))
#define GPIO_PORTJ_PUR_R        (*((volatile uint32_t *)0x40060510))
#define GPIO_PORTJ_DEN_R        (*((volatile uint32_t *)0x4006051C))
#define GPIO_PORTJ_AMSEL_R      (*((volatile uint32_t *)0x40060528))
#define GPIO_PORTJ_PCTL_R       (*((volatile uint32_t *)0x4006052C))
#define PJ0       (*((volatile uint32_t *)0x40060004))
#define PJ1       (*((volatile uint32_t *)0x40060008))
#define SWITCHES  (*((volatile uint32_t *)0x4006000C))
#define SW1_PRESSED             0x02  // value read from location SWITCHES when just SW1 is pressed
#define SW2_PRESSED             0x01  // value read from location SWITCHES when just SW2 is pressed
#define BOTH_PRESSED            0x00  // value read from location SWITCHES when both switches are pressed
#define NO_PRESSED              0x03  // value read from location SWITCHES when neither switch is pressed

#define SYSCTL_RCGCGPIO_R8      0x00000100  // GPIO Port J Run Mode Clock
                                            // Gating Control

#define SYSCTL_PRGPIO_R8        0x00000100  // GPIO Port J Peripheral Ready

void GPIO_Init(void){
                                   // activate clock for Port N
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;
                                   // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R12) == 0){};
  GPIO_PORTN_DIR_R |= 0x0F;        // make PN3-0 out
  GPIO_PORTN_AFSEL_R &= ~0x0F;     // disable alt funct on PN3-0
  GPIO_PORTN_DEN_R |= 0x0F;        // enable digital I/O on PN3-0
                                   // configure PN3-0 as GPIO
  GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTN_AMSEL_R &= ~0x0F;     // disable analog functionality on PN3-0
}
//------------PortJ_Init------------
// Initialize GPIO Port J for negative logic switches on PJ1-0
// as the Connected LaunchPad is wired.  Weak internal pull-up
// resistors are enabled.
// Input: none
// Output: none
void PortJ_Init(void){
                                   // activate clock for Port J
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R8;
                                   // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R8) == 0){};
  GPIO_PORTJ_DIR_R = 0;            // make PJ1-0 in (PJ1-0 built-in SW2-1)
  GPIO_PORTJ_AFSEL_R = 0;          // disable alt funct on PJ1-0
  GPIO_PORTJ_PUR_R = 0x03;         // enable pull-up on PJ1-0
  GPIO_PORTJ_DEN_R = 0x03;         // enable digital I/O on PJ1-0
  GPIO_PORTJ_PCTL_R = 0;           // configure PJ1-0 as GPIO
  GPIO_PORTJ_AMSEL_R = 0;          // disable analog functionality on PJ1-0
}

//------------PortJ_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 0x02 if only Switch 1 is pressed
//         0x01 if only Switch 2 is pressed
//         0x00 if both switches are pressed
//         0x03 if no switches are pressed
uint32_t PortJ_Input(void){
  return (GPIO_PORTJ_DATA_R&0x03); // read PJ1-0 inputs
}

void Switch_Init(void){
                                    // 1) activate clock for Port A
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
                                    // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R0) == 0){};
                                    // 2) no need to unlock GPIO Port A
  GPIO_PORTA_AMSEL_R &= ~0x20;      // 3) disable analog functionality on PA5
  GPIO_PORTA_PCTL_R &= ~0x00F00000; // 4) configure PA5 as GPIO
  GPIO_PORTA_DIR_R &= ~0x20;        // 5) make PA5 input
  GPIO_PORTA_AFSEL_R &= ~0x20;      // 6) disable alt funct on PA5
  GPIO_PORTA_DEN_R |= 0x20;         // 7) enable digital I/O on PA5
}

uint32_t Switch_Input2(void){
  return (GPIO_PORTA_DATA_R&0x20);  // 0x20(pressed) or 0(not pressed)
}





void delay()
{
	for(int i=0; i<12000000; i++)
	{
		
	}
}

enum states{
	OFF = 3,
	SPEED1 = 1,
	SPEED2 = 2,
	TOGGLE = 0
};

void motorOn(int speed)
{
	switch(speed)
	{
		case OFF:
			SPEED = 0;
			PWM0A_Init(60000, 0);
			break;
		case SPEED1:
			SPEED = 1;
			PWM0A_Init(60000, 15000);
			break;
		case SPEED2:
			SPEED = 2;
			PWM0A_Init(60000, 45000);
			break;
		case TOGGLE:
			SPEED = 4;
			PWM0A_Init(60000, 15000);
			delay();
			SPEED = 0;
			PWM0A_Init(60000, 45000);
			delay();
			break;
}
	}


int main(void){
	PLL_Init();                      // bus clock at 120 MHz
  PWM0A_Init(60000, 0);        // initialize PWM0A/PF0, 1000 Hz, 75% duty
  GPIO_Init();
	Switch_Init();
	PortJ_Init();
	int speed = 0;
	int flag = 0;
	//ledOn(2);
  while(1)
	{
		
		speed = PortJ_Input();
		motorOn(speed);
		
		
  }
}