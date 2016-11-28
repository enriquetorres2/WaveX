#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

#define MAGNETOMETER_HEADER_VERSION 1

//Initialize Serial Communication and MPU
void startCompass();

//Get current orientation
void getOrientation();

#endif /* MANGETOMETER_H_ */