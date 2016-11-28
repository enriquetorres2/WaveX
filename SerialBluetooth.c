#include <msp430.h>
/*
 * SerialBluetooth.c
 *
 *  Created on: Oct 25, 2016
 *      Author: Enrique J. Torres Rivera
 *  For use with the HC-06 in
 *  	slave mode Bluetooth(BT) module
 *  	connected to an MSP430F5529
 *  	LaunchPad
 *
 *  	For proper operation the BT module
 *  	can not receive the string "OK" from
 *  	an external BT device.
 *
 *  PIN ARRAY:
 *  +5V       -> Vcc BT module
 *  GND       -> GND BT module
 *  P3.3 (Tx) -> Rx BT module
 *  P3.4 (Rx) <- Tx BT module
 */
// BT connected flag
unsigned int BTConnected = 0;
unsigned int midFlag = 1;
// Initialize BT module
void initBTModule(){
	P4SEL = BIT4+BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
	//P3REN = BIT3;							// Set up resistor for BT module
	//P3OUT = BIT3;							// Make it pull up

	P5SEL |= BIT4+BIT5;                       // Select XT1

	UCSCTL6 &= ~(XT1OFF);                     // XT1 On
	UCSCTL6 |= XCAP_3;                        // Internal load cap
	UCSCTL3 = 0;                              // FLL Reference Clock = XT1

	// Loop until XT1,XT2 & DCO stabilizes - In this case loop until XT1 and DCo settle
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                    // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

	UCSCTL6 &= ~(XT1DRIVE_3);                 // Xtal is now stable, reduce drive strength

	UCSCTL4 |= SELA_0 + SELS_4 + SELM_4;      // ACLK = LFTX1
		                                      // SMCLK = default DCO
		                                      // MCLK = default DCO

	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA1CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA1BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
	UCA1BR1 = 0x00;                           //
	UCA1MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
	UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}
// Send a string to a BT connection
void sendString(char* str){
	do{
		while (!(UCA1IFG&UCTXIFG));           // USCI_A0 TX buffer ready?
		UCA1TXBUF = *str;					 // Send corresponding char
		str++;								 // Point to next char
	}while(*str);							 // Wait after NULL char
}
// Send a single character to a BT connection
void sendByte(char c){
	while (!(UCA0IFG&UCTXIFG));				 // USCI_A0 TX buffer ready?
	UCA1TXBUF = c;  							 // Send character
}
// Get BT connected flag. Affects only the BTConnected Flag.
void getConnectionStatus(){
	sendString("AT");
}
// Echo back RXed character, confirm TX buffer is ready first
// Or set/reset BTConnected Flag
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
	switch(UCA1RXBUF){
		case 'O':{
			midFlag = 1;
			break;
		}
		case 'K':{
			if(midFlag) {
				midFlag = 0;
				BTConnected = 0;
				break;
			}
		}
		default:{
			while (!(UCA1IFG&UCTXIFG));				 // USCI_A0 TX buffer ready?
			UCA1TXBUF = UCA1RXBUF;					 // Send received character
			BTConnected = 1;
		}
	}
}
