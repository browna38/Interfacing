// PWM.c
// Runs on TM4C1294
// Use M0PWM0/PF0 and M0PWM1/PF1 to generate pulse-width modulated outputs.
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

#define PWM0_ENABLE_R           (*((volatile uint32_t *)0x40028008))
#define PWM_ENABLE_PWM1EN       0x00000002  // MnPWM1 Output Enable
#define PWM_ENABLE_PWM0EN       0x00000001  // MnPWM0 Output Enable
#define PWM0_0_CTL_R            (*((volatile uint32_t *)0x40028040))
#define PWM_0_CTL_ENABLE        0x00000001  // PWM Block Enable
#define PWM0_0_LOAD_R           (*((volatile uint32_t *)0x40028050))
#define PWM0_0_COUNT_R          (*((volatile uint32_t *)0x40028054))
#define PWM0_0_CMPA_R           (*((volatile uint32_t *)0x40028058))
#define PWM0_0_CMPB_R           (*((volatile uint32_t *)0x4002805C))
#define PWM0_0_GENA_R           (*((volatile uint32_t *)0x40028060))
#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM0_0_GENB_R           (*((volatile uint32_t *)0x40028064))
#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Set the output signal to 1
#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM0_CC_R               (*((volatile uint32_t *)0x40028FC8))
#define PWM_CC_USEPWM           0x00000100  // Use PWM Clock Divisor
#define PWM_CC_PWMDIV_M         0x00000007  // PWM Clock Divider
#define PWM_CC_PWMDIV_2         0x00000000  // /2
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x4005D420))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4005D51C))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x4005D528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4005D52C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R5      0x00000020  // GPIO Port F Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCPWM_R        (*((volatile uint32_t *)0x400FE640))
#define SYSCTL_RCGCPWM_R0       0x00000001  // PWM Module 0 Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R5        0x00000020  // GPIO Port F Peripheral Ready
#define SYSCTL_PRPWM_R          (*((volatile uint32_t *)0x400FEA40))
#define SYSCTL_PRPWM_R0         0x00000001  // PWM Module 0 Peripheral Ready

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PF0 and PF1 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 120 MHz/2 = 60 MHz (in this example)
// Output on PF0/M0PWM0
void PWM0A_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;// 1) activate clock for PWM0
                                        // 2) activate clock for Port F
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
                                        // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};
  GPIO_PORTF_AFSEL_R |= 0x01;           // 3) enable alt funct on PF0
  GPIO_PORTF_DEN_R |= 0x01;             //    enable digital I/O on PF0
                                        // 4) configure PF0 as PWM0
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFFF0)+0x00000006;
  GPIO_PORTF_AMSEL_R &= ~0x01;          //    disable analog functionality on PF0
                                        // allow time for clock to stabilize
  while((SYSCTL_PRPWM_R&SYSCTL_PRPWM_R0) == 0){};
  PWM0_CC_R |= PWM_CC_USEPWM;           // 5) use PWM divider
  PWM0_CC_R &= ~PWM_CC_PWMDIV_M;        //    clear PWM divider field
  PWM0_CC_R += PWM_CC_PWMDIV_2;         //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 6) re-loading down-counting mode
  PWM0_0_GENA_R = (PWM_0_GENA_ACTCMPAD_ONE|PWM_0_GENA_ACTLOAD_ZERO);
  // PF0 goes low on LOAD
  // PF0 goes high on CMPA down
  PWM0_0_LOAD_R = period - 1;           // 7) cycles needed to count down to 0
  PWM0_0_CMPA_R = duty - 1;             // 8) count value when output rises
  PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;     // 9) start PWM0
  PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;   // 10) enable PWM0A/PF0 outputs
}

// change duty cycle of PWM0A/PF0
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0A_Duty(uint16_t duty){
  PWM0_0_CMPA_R = duty - 1;             // 8) count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PF0 and PF1 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 120 MHz/2 = 60 MHz (in this example)
// Output on PF1/M0PWM1
void PWM0B_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;// 1) activate clock for PWM0
                                        // 2) activate clock for Port F
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
                                        // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};
  GPIO_PORTF_AFSEL_R |= 0x02;           // 3) enable alt funct on PF1
  GPIO_PORTF_DEN_R |= 0x02;             //    enable digital I/O on PF1
                                        // 4) configure PF1 as PWM0
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFFF0F)+0x00000060;
  GPIO_PORTF_AMSEL_R &= ~0x02;          //    disable analog functionality on PF1
                                        // allow time for clock to stabilize
  while((SYSCTL_PRPWM_R&SYSCTL_PRPWM_R0) == 0){};
  PWM0_CC_R |= PWM_CC_USEPWM;           // 5) use PWM divider
  PWM0_CC_R &= ~PWM_CC_PWMDIV_M;        //    clear PWM divider field
  PWM0_CC_R += PWM_CC_PWMDIV_2;         //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 6) re-loading down-counting mode
  PWM0_0_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE|PWM_0_GENB_ACTLOAD_ZERO);
  // PF1 goes low on LOAD
  // PF1 goes high on CMPB down
  PWM0_0_LOAD_R = period - 1;           // 7) cycles needed to count down to 0
  PWM0_0_CMPB_R = duty - 1;             // 8) count value when output rises
  PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;     // 9) start PWM0
  PWM0_ENABLE_R |= PWM_ENABLE_PWM1EN;   // 10) enable PWM0B/PF1 outputs
}

// change duty cycle of PWM0B/PF1
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0B_Duty(uint16_t duty){
  PWM0_0_CMPB_R = duty - 1;             // 8) count value when output rises
}
