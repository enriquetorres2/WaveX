#include <msp430.h>
#define FRONT_RANGE 18
#define SIDE_RANGE 9
/*
 * proximity.c
 *
 *  Created on: Nov 5, 2016
 *      Author: enriquetorres
 */
unsigned int buff1[2];
unsigned int buff2[2];
unsigned int buff3[2];
unsigned short leftFlag;
unsigned short rightFlag;
unsigned short frontFlag;
void initProximitySersors(){
	//Buffers to hold values
	buff1[0] = 0; // To store time value at falling edge for right sensor echo.
	buff1[1] = 0; // To store time value at rising edge for right sensor echo.
	buff2[0] = 0; // To store time value at falling edge for left sensor echo.
	buff2[1] = 0; // To store time value at rising edge for left sensor echo.
	buff3[0] = 0; // To store time value at falling edge for front sensor echo.
	buff3[1] = 0; // To store time value at rising edge for front sensor echo.

	// Configure PWM signal for triggering proximity sensor
	P1SEL |= BIT5; // Prepare pin for PWM output (P1.5)
	TA0CCR0 = 65535; // 4Hz signal
	TA0CCTL0 |= CCIE; // Set interrupt for refreshing values
	TA0CCTL4 |= OUTMOD_7; // Set PWM output to pin
	TA0CCR4 = 1; // Set Pulse Width to 10ns
	TA0CTL |= TACLR; // Clear TA0CLK
	TA0CTL = TASSEL_2+MC_1+ID_2; //Start Timer A0 with SMCLK source (1,048,576 Hz), Up mode and a prescalar of 4.

	P2DIR &= ~(BIT3+BIT4+BIT5); // Configure Pins for input (P2.3-5)
	P2SEL |= BIT3+BIT4+BIT5; // Set pins as capture mode for Timer A2.
	TA2CCTL0 = CM_1+CCIS_0+CAP+CCIE; // Configure Capture mode of P2.3 and enable interrupt for rising edge.
	TA2CCTL1 = CM_1+CCIS_0+CAP+CCIE; // Configure Capture mode of P2.4 and enable interrupt for rising edge.
	TA2CCTL2 = CM_1+CCIS_0+CAP+CCIE; // Configure Capture mode of P2.5 and enable interrupt for rising edge.
	TA2EX0 = TAIDEX_6; // Set prescalar to divide the clock by 7.
	TA2CTL |= TACLR; // Clear TA2CLK
	TA2CTL = TASSEL_1+MC_2; //Start Timer A2 with ACLK source (32,768 Hz) and continuous mode.
}
void checkProximity(){
	int val1 = buff1[0]-buff1[1]; // Calculate the difference between negative edge and positive edge of the echo signal from right sensor.
	if(val1 <= SIDE_RANGE){ // Compare difference with the range.
		rightFlag |= BIT0; //Set RightFlag
	}else{
		rightFlag &= ~BIT0; //Clear RightFlag
	}
	int val2 = buff2[0]-buff2[1]; // Calculate the difference between negative edge and positive edge of the echo signal from left sensor.
	if(val2 <= SIDE_RANGE){ // Compare difference with the range.
		leftFlag |= BIT0; //Set LeftFlag
	}else{
		leftFlag &= ~BIT0; //Clear LeftFlag
	}
	int val3 = buff3[0]-buff3[1]; // Calculate the difference between negative edge and positive edge of the echo signal from front sensor.
	if(val3 <= FRONT_RANGE){ // Compare difference with the range.
		frontFlag |= BIT0; //Set FrontFlag
	}else{
		frontFlag &= ~BIT0; //Clear FrontFlag
	}
	TA2R = 0; // Clear TA2 counter
}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void Sensors(void){ // Interrupt for right sensor.
	if(TA2CCTL0 & CM_1){ // If set for rising edge
		buff1[1] = TA2CCR0; // Record rising edge value.
		TA2CCTL0 &= ~CM_1; // Clear rising edge interrupt.
		TA2CCTL0 |= CM_2; // Set falling edge interrupt.
	}
	else if(TA2CCTL0 & CM_2){ // If set for falling edge
		buff1[0] = TA2CCR0; // Record falling edge value.
		TA2CCTL0 &= ~CM_2; // Clear falling edge interrupt.
		TA2CCTL0 |= CM_1; // Set rising edge interrupt.
	}

}
#pragma vector = TIMER2_A1_VECTOR //1 & 2
__interrupt void Sensor2(void){ // Interrupt for left and front sensor.
	switch (TA2IV){
		case 0x02:{ // For left sensor
			if(TA2CCTL1 & CM_1){ // If set for rising edge
					buff2[1] = TA2CCR1; // Record rising edge value.
					TA2CCTL1 &= ~CM_1; // Clear rising edge interrupt.
					TA2CCTL1 |= CM_2; // Set falling edge interrupt.
				}
				else if(TA2CCTL1 & CM_2){ // If set for falling edge
					buff2[0] = TA2CCR1; // Record falling edge value.
					TA2CCTL1 &= ~CM_2; // Clear falling edge interrupt.
					TA2CCTL1 |= CM_1; // Set rising edge interrupt.
				}
			break;
		}
		case 0x04:{ // For front sensor
			if(TA2CCTL2 & CM_1){ // If set for rising edge
					buff3[1] = TA2CCR2; // Record rising edge value.
					TA2CCTL2 &= ~CM_1; // Clear rising edge interrupt.
					TA2CCTL2 |= CM_2; // Set falling edge interrupt.
			}
			else if(TA2CCTL2 & CM_2){ // If set for falling edge
				buff3[0] = TA2CCR2; // Record falling edge value.
				TA2CCTL2 &= ~CM_2; // Clear falling edge interrupt.
				TA2CCTL2 |= CM_1; // Set rising edge interrupt.
			}
			break;
		}
		default:;
		// Do nothing for false interrupts
	}
}
