#include <msp430.h> 
#include "magnetometer.h"
#include "multimeter.h"
#include "proximity.h"
#include "SerialBluetooth.h"

#define forward 0            	// For easier developing
#define left 1                     // here are a few values
#define right 2
#define back 3
#define stop 4
#define manual 0
#define automatic 1
#define connected 1
#define disconnected 0

int dirrection;       // 0 = forward, 1 = left, 2 = right, 3 = back, 4 = stop
int turn;                	// Turning flag, 0 turn left, 1 turn right
int mode;                // 0 = Manual, 1 Automatic


void turn_right(){
	P1OUT &= ~(BIT2 + BIT3);                                                             // Clear last input
	P8OUT &= ~(BIT1 + BIT2);                                                             // Clear last input
	P1OUT |= BIT2;                                                                       // Set the relays to turn counterclockwise
	P8OUT |= BIT1;                                                                       //
}
void turn_left(){
	P1OUT &= ~(BIT2 + BIT3);                                                             // Clear last input
	P8OUT &= ~(BIT1 + BIT2);                                                             // Clear last input
	P1OUT |= BIT3;                                                                       //Set the relay to turn clockwise
	P8OUT |= BIT2;
}
void go_forward(){
	P1OUT &= ~(BIT2 + BIT3);                                                             // Clear last input
	P8OUT &= ~(BIT1 + BIT2);                                                             // Clear last input
	P1OUT |= BIT3;
	P8OUT |= BIT1;
}
void go_backwards(){
	P1OUT &= ~(BIT2 + BIT3);                                                             // Clear last input
	P8OUT &= ~(BIT1 + BIT2);                                                             // Clear last input
	P1OUT |= (BIT2);
	P8OUT |= (BIT2);
}
void off(){
	P1OUT &= ~(BIT2 + BIT3);                                                             // Clear last input
	P8OUT &= ~(BIT1 + BIT2);                                                             // Clear last input
}
/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;               // Stop WatchDog Timer

	// Initialize all ports
	P1OUT = 0;
	P2OUT = 0;
	P3OUT = 0;
	P4OUT = 0;
	P5OUT = 0;
	P6OUT = 0;
	P7OUT = 0;
	P8OUT = 0;
	PJOUT = 0;

	P1DIR = 0xFF;
	P2DIR = 0xFF;
	P3DIR = 0xFF;
	P4DIR = 0xFF;
	P5DIR = 0xFF;
	P6DIR = 0xFF;
	P7DIR = 0xFF;
	P8DIR = 0xFF;
	PJDIR = 0xFF;

	//Initialize global variables
	dirrection = stop;
	turn = -1; //Do not turn
	mode = manual; //Default mode is manual

	//Initialize all Modules
	initBTModule(); // Bluetooth Module
	initProximitySersors(); // Proximity Sensors
	initMultiMeter(); // Battery and Current Monitor
	startCompass(); // Magnetometer

	_BIS_SR(GIE); // Enable Interrupts
	P1OUT |= BIT2;   // Set system operational

	//Start main program loop
	while(1){
		//BLUETOOTH
		//Receive bluetooth commands/request from the app
		//and executes/answers

		BTBuffer = 'U';
		getConnectionStatus();
		__delay_cycles(200);
		if(BTConnected == disconnected){//Is the bluetooth connected?
				P2OUT &= ~BIT0;//Turn off Bluetooth LED   TODO
		}
		else{// It is connected
			P4OUT |= BIT3;// Turn on Bluetooth LED
			// Interpretation of instruction
			if(BTBuffer == 'm'){                   	  // Tell app what mode WaveX is in.
				if(mode == automatic){                 // If auto display manual button
					sendByte('n');
				}
				else{                                  // If manual display auto button
					sendByte('b');
				}
			}
			else if(BTBuffer == 'v'){//Tell app the battery %
				doSamples();
				if(battery > 80){//Is battery more than 80%?
					sendByte('l');//If
				}
				else if(battery > 60){//Is battery more than 60%?
					sendByte('k');
				}
				else if(battery > 40){//Is battery more than 40%?
					sendByte('j');
				}
				else if(battery > 20){//Is battery more than 20%?
					sendByte('h');
				}
				else if(battery >  0){//Is battery more than 0%?
					sendByte('g');
				}
				else{
					//lowBatteryMode();
				}
			}
			if(mode == manual){//Control
				if(BTBuffer == 'w'){//Go forward?
					dirrection = forward;
				}
				else if(BTBuffer == 'a'){//Go left?
					dirrection = left;
				}
				else if(BTBuffer == 's'){//Go in back?
					dirrection = back;
				}
				else if(BTBuffer == 'd'){//Go right?
					dirrection = right;
				}
				else if(BTBuffer == 't'){//Stop?
					dirrection = stop;
				}
			}
			if(BTBuffer == 'p'){         //Should we go into manual?
				if(mode != manual){      //if already in manual do not stop motors
					dirrection = stop;   //Stop when entering manual
				}
				mode = manual;           //Set mode to manual
			}
			else if(BTBuffer == 'o'){    //Should we go into auto?
				mode = automatic;        //Set mode to auto
			}
		}

		//NAV CONTROL
		//Here the MCU picks which direction
		if(mode == manual){       		//Is it in manual?
			P4OUT &= ~BIT0;				//Turn off Mode LED
		}
		else{							//Its in automatic
			P4OUT |= BIT0;				//Turn on Mode LED

			//Navigation algorithm
			if(frontFlag == 1){			//The front if blocked
				if(leftFlag == 1){		//The left is blocked
					dirrection = right;
				}
				else if(rightFlag == 1){//The right is blocked
					dirrection = left;
				}
				else{					//Neither side is blocked
					dirrection = stop;
				}
			}
			else{						//Nothing blocking the way keep going
				dirrection = forward;
			}
		}
		if(dirrection == forward){		//Do we go forward?
			//Power Both Engines
			go_forward();
		}
		else if(dirrection == back){		//Do go back?
			//Shutdown both engines
			go_backwards();
		}
		else if(dirrection == stop){		//Do we stop?
			//Shutdown both engines
			off();
		}
		else if(dirrection == left || dirrection == right){//Do we turn?
			if(dirrection == left){						//Do we turn left?
				//Shutdown left engine
				turn_left();
			}
			else{//We turn right
				//Shutdown right engine
				turn_right();
			}
		}
	}
}
