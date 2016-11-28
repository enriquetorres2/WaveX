#include <msp430.h> 
/*
 *
 */
static unsigned int index;
unsigned int batteryValue;
unsigned int currentValue;
volatile unsigned int A0results[NUM_OF_RESULTS];
volatile unsigned int A2results[NUM_OF_RESULTS];
void initMultiMeter() {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    index = 0; //Initialize index

    // Set pin to activate battery monitor reading
    P6DIR |= BIT1;
    P6OUT |= BIT1;

    // Set ports to ADC
	P6DIR &= ~(BIT0+BIT2);
	P6SEL |= BIT0+BIT2;
	
	ADC12CTL0 = ADC12MSC + ADC12SHT0_8 + ADC12ON;// Extended sampling time, ADC12 on
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;  // Use sampling timer
	ADC12MCTL0 |= 0x00;						 // Channel A0
	ADC12MCTL1 |= 0x02+ADC12EOS;				 // Channel A2 and set as last channel
	ADC12IE = 0x04;                           // Enable interrupt for channel A2
	ADC12CTL0 |= ADC12ENC;                    // Enable conversion
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
		A0results[index] = ADC12MEM0; // Record on memory the value for current monitor
		A2results[index] = ADC12MEM1; // Record on memory the value for battery monitor
		index++; // Increment index
		if (index == 8) index = 0;
		batteryValue = average(A2results);//Battery low flag
		currentValue = average(A0results); // Current value
	}
}