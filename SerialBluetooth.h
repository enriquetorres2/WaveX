/*
 * SerialBluetooth.h
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

#ifndef SERIALBLUETOOTH_H_
#define SERIALBLUETOOTH_H_

#define SERIALBLUETOOTH_HEADER_VERSION 1

// BT connected flag
extern unsigned int BTConnected;
// Initialize BT module
void initBTModule();
// Send a string to a BT connection
void sendString(char* str);
// Send a single character to a BT connection
void sendByte(char c);
// Get BT connected flag. Affects only the connection flag.
void getConnectionStatus();

#endif /* SERIALBLUETOOTH_H_ */
