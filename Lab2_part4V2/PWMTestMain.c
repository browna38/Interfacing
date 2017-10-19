// PWMTestMain.c
// Runs on TM4C1294
// Use PWM0A/PF0 and PWM0B/PF1 to generate pulse-width modulated outputs.
// Daniel Valvano
// June 19, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
  Program 6.7, section 6.3.2

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

#include <stdint.h>
#include "PLL.h"
#include "PWM.h"

void WaitForInterrupt(void);  // low power mode

int main(void){
  PLL_Init();                      // bus clock at 120 MHz
  PWM0A_Init(60000, 45000);        // initialize PWM0A/PF0, 1000 Hz, 75% duty
  PWM0B_Init(60000, 15000);        // initialize PWM0B/PF1, 1000 Hz, 25% duty
  PWM0A_Duty(6000);    // 10%
  PWM0A_Duty(15000);   // 25%
	
  PWM0A_Duty(45000);   // 75%

//  PWM0A_Init(6000, 3000);           // initialize PWM0A/PF0, 10000 Hz, 50% duty
//  PWM0A_Init(1500, 1350);           // initialize PWM0A/PF0, 40000 Hz, 90% duty
//  PWM0A_Init(1500, 225);            // initialize PWM0A/PF0, 40000 Hz, 15% duty
//  PWM0A_Init(60, 30);               // initialize PWM0A/PF0, 1 MHz, 50% duty
  while(1){
    WaitForInterrupt();
		
  }
}
