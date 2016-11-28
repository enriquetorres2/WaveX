#include <msp430.h> 
#include "magnetometer.h"
#include "multimeter.h"
#include "proximity.h"
#include "SerialBluetooth.h"
/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}
