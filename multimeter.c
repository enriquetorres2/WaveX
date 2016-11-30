#include <msp430.h> 
#define BATTERY_CONST (100/620) // Set constant value to converts volts to percent
#define BATTERY_OFFSET 3110 // Set offset to minimum battery operation allowed
#define CURRENT_CONST 1 // Set constant according to Ohms Law
static unsigned int index;
int battery;
unsigned int current;
volatile unsigned int A0results[8];
volatile unsigned int A1results[8];
void initMultiMeter() {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    index = 0; //Initialize index

    // Set pin to activate battery monitor reading
    P6DIR |= BIT2;
    P6OUT |= BIT2;

    // Set ports to ADC
	P6DIR &= ~(BIT0+BIT1);
	P6SEL |= BIT0+BIT1;
	
	ADC12CTL0 = ADC12MSC + ADC12SHT0_8 + ADC12ON;// Extended sampling time, ADC12 on
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_1;  // Use sampling timer
	ADC12MCTL0 |= 0x00;						 // Channel A0
	ADC12MCTL1 |= 0x01+ADC12EOS;				 // Channel A1 and set as last channel
	ADC12IE = 0x02;                           // Enable interrupt for channel A1
	ADC12CTL0 |= ADC12ENC;                    // Enable conversion
}
void doSamples(){
	ADC12CTL0 |= ADC12SC;					 // Start conversion - software trigger
}
int average(volatile unsigned int a[8]){// Calculate average for all values in array
	unsigned av;
	unsigned int i;
	for(i=0;i<8;i++){
		av+=a[i];
	}
	return av >> 3;
}
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR(void){
	if(ADC12IV & 8){ // Double check source of interrupt
		ADC12IFG = 0;
		A0results[index] = ADC12MEM0; // Record on memory the value for current monitor
		A1results[index] = ADC12MEM1; // Record on memory the value for battery monitor
		index++; // Increment index
		if (index == 8) index = 0;
		battery = (average(A1results)-BATTERY_OFFSET)*BATTERY_CONST; //Battery Value in percent
		current = average(A0results)*CURRENT_CONST; // Current value
	}
}
