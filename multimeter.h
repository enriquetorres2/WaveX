#ifndef MULTIMETER_H_
#define MULTIMETER_H_

#define MULTIMETER_HEADER_VERSION 1

//Generated Values
extern int battery;
extern unsigned int current;

// Initialize Multimeter
void initMultiMeter();
// Sample measurements
void doSamples();

#endif /* MULTIMETER_H_ */
