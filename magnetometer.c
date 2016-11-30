#include <msp430.h>
#include "math.h"

#define X_COMPONENT_OFFSET -128
#define Y_COMPONENT_OFFSET -335
#define RADIAN_DEGREES_FACTOR 180.0/3.141593

float sensVal[3];
int buff;									//input buffer
float orientation;
int receiveFlag;
int x, ax;
int y, ay;
int z, az;
float mx;
float my;
float mz;

unsigned int TXData;
unsigned char TXByteCtr;
int Rx;
void transmit(int word){						// Transmit to slave
	Rx = 0;
	TXData = word;
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0 w/ interrupts
}
void receive(void){							// Receive from slave

    Rx = 1;
	__bic_SR_register(GIE);
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 &= ~UCTR ;                     // Set UCTR for reading
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
	//__delay_cycles(1000);					// Wait
	UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	__bis_SR_register(LPM0_bits + GIE);     // Enter LPM0 w/ interrupts
	__no_operation();

}


void i2cSetup(){

  P3SEL |= BIT0 + BIT1;                            // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
  UCB0BR1 = 0;
  UCB0I2CSA = 0x68;                         // Slave Address is 12 for magnet
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0IE |= UCRXIE + UCTXIE;

}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void initCompass(){
	UCB0I2CSA = 0x68;// Slave Address for MPU
	//Each block represent a transaction between the MCU and the MPU-9265
	TXByteCtr = 2;
	Rx = 0;
	__no_operation();//Reset system 1
	transmit(0x006B);
	__no_operation();
	__delay_cycles(10);

	TXByteCtr = 2;
	Rx = 0;
	__no_operation();//Reset system 2
	transmit(0x006C);
	__no_operation();
	__delay_cycles(10);

	TXByteCtr = 2;
	Rx = 0;
	__no_operation();//Set bypass to access magnetometer from MCU
	transmit(0x0237);
	__no_operation();

	UCB0I2CSA = 0x0C;// Slave Address for magnetometer

	TXByteCtr = 2; // Power down magnetometer
	__no_operation();
	transmit(0x000A);
	__no_operation();
	__delay_cycles(10);

	TXByteCtr = 2;
	__no_operation();//Enter magnetometer fuse Fuse ROM access Mode
	transmit(0x0F0A);
	__no_operation();
	__delay_cycles(10);

	TXByteCtr = 1;
	receiveFlag = 0;
	__no_operation();//Read x calibration value
	transmit(0x10);
	__no_operation();
	Rx = 1;
	__no_operation();
	receive();
	__no_operation();

	sensVal[0]=(float)((buff-128)/256.0)+1.0; // Record x calibration value

	TXByteCtr = 1;
	receiveFlag = 0;
	__no_operation();//Read y calibration value
	transmit(0x11);
	__no_operation();
	Rx = 1;
	__no_operation();
	receive();
	__no_operation();

	sensVal[1]=(float)((buff-128)/256.0)+1.0; // Record y calibration value

	TXByteCtr = 1;
	receiveFlag = 0;
	__no_operation();//Read z calibration value
	transmit(0x12);
	__no_operation();
	Rx = 1;
	__no_operation();
	receive();
	__no_operation();

	sensVal[2]=(float)((buff-128)/256.0)+1.0; // Record z calibration value

	receiveFlag = 1;
	TXByteCtr = 2; // Power down magnetometer
	__no_operation();
	transmit(0x000A);
	__no_operation();
	__delay_cycles(10);

	TXByteCtr = 2;
	__no_operation();//16 bit output and 8hz measurement mode
	transmit(0x120A);
	__no_operation();
	__delay_cycles(10);
}
void tallyOrientation(){

	x = 0;
	y = 0;
	z = 0;
	ax = 0;
	ay = 0;
	az = 0;

	int i;
	for (i = 0; i < 8; i++){
		initCompass();
		//Each block represent a transaction between the MCU and the MPU-9265
		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read X High byte value
		transmit(0x04);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		x |= buff;
		x = x << 8;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read X Low byte value
		transmit(0x03);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		x |= buff;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read Y High byte value
		transmit(0x06);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		y |= buff;
		y = y << 8;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read Y Low byte value
		transmit(0x05);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		y |= buff;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read Z High byte value
		transmit(0x08);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		z |= buff;
		z = z << 8;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read Z Low byte value
		transmit(0x07);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		z |= buff;

		TXByteCtr = 1;
		Rx = 0;
		__no_operation();//Read Z Low byte value
		transmit(0x0A);
		__no_operation();
		Rx = 1;
		__no_operation();
		receive();
		__no_operation();

		//Add up to get average
		ax += x; 
		ay += y; 
		az += z; 
	}
	// Calculate average
	ax = ax >> 3;
	ay = ay >> 3;
	az = az >> 3;

	mx = (float)ax*(10.0*4912.0/32760.0)*sensVal[0] + X_COMPONENT_OFFSET; // Get actual x magnetometer value;
	my = (float)ay*(10.0*4912.0/32760.0)*sensVal[1] + Y_COMPONENT_OFFSET; // Get actual y magnetometer value;
	mz = (float)az*(10.0*4912.0/32760.0)*sensVal[2];// - 125.0; // Get actual z magnetometer value;
}
void calculateOrientation(){
	//For Calculate Orientation use the following formula
	//derived from the dot product
	//orientation = acos(x/sqrt((x*x)+(y*y)))

	float mx2 = (long) mx*mx;
	float my2 = (long) my*my;
	float m2 = my2+mx2;
	float m = sqrtf(m2);
	float nm = 1/m;
	if(my<=0) orientation = acosf(mx*nm);
	else orientation = -1.0*acosf(mx*nm);
	if(orientation == NAN) orientation = 0.0;
	else orientation = orientation *RADIAN_DEGREES_FACTOR;
}

void startCompass(){
  receiveFlag = 1;
  i2cSetup();
}

float getOrientation(){
	tallyOrientation();
	calculateOrientation();
	__no_operation();
	return orientation;
}
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
  if(Rx){ // Check if waiting to receive an byte
	  buff = UCB0RXBUF; // Store to buffer the byte received
	  UCB0IFG &= ~UCRXIFG; // Clear Interrupt Flag
	  __bic_SR_register_on_exit(LPM0_bits);
  }
  else{
	switch (TXByteCtr){// Check TX byte counter
	case 2:{
		UCB0TXBUF = (char) TXData;            // Load TX buffer
		TXByteCtr--;                          // Decrement TX byte counter
		TXData = TXData >> 8;				 // Prepare to send the next flag
		break;
	}
	case 1:{
	  UCB0TXBUF = (char) TXData;              // Load TX buffer
	  TXByteCtr--;                            // Decrement TX byte counter
	  break;
	}
	case 0:{
	  if(receiveFlag) UCB0CTL1 |= UCTXSTP;   // Send I2C stop condition flag if is not receiving a value.
	  UCB0IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
	  __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
	  break;
	}
	default:break;
	}
  }
}
