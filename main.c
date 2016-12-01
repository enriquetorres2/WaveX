#include <msp430.h> 
#include "magnetometer.h"
#include "multimeter.h"
#include "proximity.h"
#include "SerialBluetooth.h"

/*
 * Wiring
 * Motors--------------------------------------------
 * RIGHT
 * P1.2 -> purple
 * P1.3 -> blue
 * LEFT
 * P8.2 -> grey
 * P8.3 -> white
 *
 * Bluetooth UART------------------------------------
 * P3.4(RX) -> TX
 * P3.3(TX) -> RX
 *
 * LEDS ---------------------------------------------
 * LED A - POWER - P2.0
 * LED B - SYSTEM OK - P2.2
 * LED C - MODE LED - P4.0
 * LED D - Bluetooth LED - P4.3
 *
 *
 * ADC PINS -----------------------------------------
 *
 * Battery Mon. Enable - P6.2
 * Battery Mon. Read - P6.1
 * Multimeter IN - P6.0
 *
 *
 * MISC ---------------------------------------------
 *
 * Buzzer Enable - P3.7
 *
 *
 */


#define forward 0            	// For easier developing
#define left 1                     // here are a few values
#define right 2
#define back 3
#define stop 4
#define manual 0
#define automatic 1
#define connected 1
#define disconnected 0

#define LEDA_ON P2OUT |= BIT0
#define LEDB_ON P2OUT |= BIT2
#define LEDC_ON P4OUT |= BIT0
#define LEDD_ON P4OUT |= BIT3
#define ENABLE_BUZZ P3OUT |= BIT7


#define LEDA_OFF P2OUT &= ~BIT0
#define LEDB_OFF P2OUT &= ~BIT2
#define LEDC_OFF P4OUT &= ~BIT0
#define LEDD_OFF P4OUT &= ~BIT3
#define DISABLE_BUZZ P3OUT &= ~BIT7


int direction;       	// 0 = forward, 1 = left, 2 = right, 3 = back, 4 = stop
int turn;               // Turning flag, 0 turn left, 1 turn right
int mode;               // 0 = Manual, 1 Automatic

void turn_left(){
	P1OUT &= ~(BIT2 + BIT3);				// Clear last input
	P8OUT &= ~(BIT1 + BIT2);				// Clear last input
	P1OUT |= BIT2;							// Set the relays to turn counterclockwise
	P8OUT |= BIT1;							//
}

void turn_right(){
	P1OUT &= ~(BIT2 + BIT3);				// Clear last input
	P8OUT &= ~(BIT1 + BIT2);				// Clear last input
	P1OUT |= BIT3;								//Set the relay to turn clockwise
	P8OUT |= BIT2;
}
void go_forward(){
	P1OUT &= ~(BIT2 + BIT3);				// Clear last input
	P8OUT &= ~(BIT1 + BIT2);				// Clear last input
	P1OUT |= BIT2;
	P8OUT |= BIT2;
}
void go_backwards(){
	P1OUT &= ~(BIT2 + BIT3);				// Clear last input
	P8OUT &= ~(BIT1 + BIT2);				// Clear last input
	P1OUT |= (BIT3);
	P8OUT |= (BIT1);
}
void off(){
	P1OUT &= ~(BIT2 + BIT3);				// Clear last input
	P8OUT &= ~(BIT1 + BIT2);				// Clear last input
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
	direction = stop;
	turn = -1; //Do not turn
	mode = manual; //Default mode is manual

	//Initialize all Modules
	initBTModule(); // Bluetooth Module
	initProximitySersors(); // Proximity Sensors
	//initMultiMeter(); // Battery and Current Monitor
	startCompass(); // Magnetometer

	//Initialize switch value
	P3DIR &= ~BIT6;
	__delay_cycles(10000); // Warm up the SYSTEM
	unsigned short toogleSwitch = P3IN & BIT6;
	unsigned short turnDir = 1;
	//Initialize Orientation variables
	float orientation = -1; 		// Boat orientation
	float newOrientation = -1;	// New boat orientation

	_BIS_SR(GIE); // Enable Interrupts
	P1OUT |= BIT2;   // Set system operational

	//Start main program loop
	while(1){
		sendString(".");
		//Hardware switch
		if(toogleSwitch != (P3IN & BIT6)){ //Toogle between modes regardless the position of the switch
			toogleSwitch = P3IN & BIT6;
			if(mode){
				sendString("ch to user control\n\r");
				mode = manual;
				direction = stop;
			}else{
				sendString("ch to auto\n\r");
				mode = automatic;
			}
		}

		//BLUETOOTH
		//Receive bluetooth commands/request from the app
		//and executes/answers

//		BTBuffer = 'U';
//		//		getConnectionStatus();
//		__delay_cycles(200);
//		if(BTConnected == disconnected){//Is the bluetooth connected?
//			P2OUT &= ~BIT0;//Turn off Bluetooth LED   TODO
//		}
		//else{// It is connected
			LEDD_ON; // Turn on Bluetooth LED
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
				//doSamples();
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
					//lowBatteryMode;
				}
			}
			if(mode == manual){//Control
				sendString("user control select\n\r");
				if(BTBuffer == 'w'){//Go forward?
					direction = forward;
					sendString("Select Forward\n\r");
				}
				else if(BTBuffer == 'a'){//Go left?
					direction = left;
					sendString("Select Left\n\r");
				}
				else if(BTBuffer == 's'){//Go in back?
					direction = back;
					sendString("Select reverse\n\r");
				}
				else if(BTBuffer == 'd'){//Go right?
					direction = right;
					sendString("Select Right\n\r");
				}
				else if(BTBuffer == 't'){//Stop?
					direction = stop;
					sendString("Select Stop\n\r");
				}
			}
			if(BTBuffer == 'p'){         //Should we go into manual?
				if(mode != manual){      //if already in manual do not stop motors
					direction = stop;   //Stop when entering manual
					sendString("Select Stop due user control\n\r");
				}
				mode = manual;           //Set mode to manual
				sendString("Select Stop due user control\n\r");
			}
			else if(BTBuffer == 'o'){    //Should we go into auto?
				mode = automatic;        //Set mode to auto
				sendString("Go auto\n\r");
			}
		//}

		//NAV CONTROL
		//Here the MCU picks which direction
		if(mode == manual){       		//Is it in manual?
			P4OUT &= ~BIT0;				//Turn off Mode LED
		}
		else{							//Its in automatic
			P4OUT |= BIT0;				//Turn on Mode LED
			//Navigation algorithm
			if(frontFlag == 1){			//The front if blocked
				sendString("FFlag - get orie.\n\r");
				orientation = getOrientation();
				newOrientation = orientation;
				sendString("Got orie.\n\r");
				if(leftFlag == 1){		//The left is blocked
					sendString("LeftFlag\n\r");
					direction = right;
				}
				else if(rightFlag == 1){//The right is blocked
					sendString("RightFlag\n\r");
					direction = left;
				}
				else{					//Neither side is blocked
					sendString("default t.\n\r");
					if(turn){
						direction = left;
						turn = 0;
					}else{
						direction = right;
						turn = 1;
					}
				}
			}
			else{ //Nothing blocking the way keep going
				direction = forward;
				sendString("keep go\n\r");
			}
		}
		if(direction == forward){		//Do we go forward?
			//Power Both Engines
			go_forward();
			sendString("go_forward\n\r");
		}
		else if(direction == back){		//Do go back?
			//Shutdown both engines
			go_backwards();
			sendString("go_reverse\n\r");
		}
		else if(direction == stop){		//Do we stop?
			//Shutdown both engines
			off();
			sendString("stop\n\r");
		}
		else if(direction == left || direction == right){//Do we turn?
			if(direction == left){						//Do we turn left?
				//Shutdown left engine
				turn_left();
				sendString("t_left\n\r");
			}
			else{//We turn right
				//Shutdown right engine
				turn_right();
				sendString("t_right\n\r");
			}
			if((orientation > -1) && (mode) ){
				sendString("E 180 degree t\n\r");
				int turnDeg = newOrientation - orientation;
				while(turnDeg < 170){
					newOrientation = getOrientation();
					turnDeg = newOrientation - orientation;
					if(turnDeg < 0){
						turnDeg = -1*turnDeg;
					}
					if(BTBuffer == 'p') break;
				}
				orientation = -1;
				newOrientation = -1;
				direction = forward;
				sendString("Ex 180 degree t\n\r");
			}

		}
	}
}
