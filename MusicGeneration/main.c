/*
--------------------------------------------
Cory Pisano
April 22, 2014
ECE:435   - Embedded Systems Programming
Project 2 - Tone and Music Generation
--------------------------------------------
Assignment Instructions:
     . . .

Switching between modes - interrupt driven? or in while(1) { }
 */

#include <msp430.h>

/* globals */
#define INPUT 		BIT3
#define LEDR 		BIT0
#define LEDG 		BIT6
#define CLOCK_F 	16000000
#define BIN_WIDTH 	255
#define FREQ_SCALAR	CLOCK_F/(4*BIN_WIDTH)

//volatile unsigned int volume 	= 8;  // inverse volume (2 is max, 255 is min)
volatile unsigned int toneCount = 36; //
volatile unsigned int count 	= 0;	  //

/* Timer A0 timeout interrupt
 * occurs when TAR == BIN_WIDTH
 *
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA0 (void) {

	count++;

	/* SET TA0CCR1 */
	if (count < toneCount/2) {
		TA0CCR1 = 255;
	}
	else if (count < toneCount) {
		TA0CCR1 = 0;
	}

	/* Reset count */
	if (count == toneCount) {
		count = 0;
	}

	TA0CTL &= ~TAIFG;
}

/* freqToCount function
 * input  - frequency in Hertz
 * sets global variable toneCount
 */
void playTone(unsigned int f){
	toneCount = FREQ_SCALAR/f;
}

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog timer

	/* Clock Setup - 16MHz */
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL  = CALDCO_16MHZ;

	/* TIMER A0 SETUP */
	TA0CTL	 = MC_1 | ID_2 | TASSEL_2;  // divide by 4
	TA0CCTL0 = CCIE;                    // Enable Timer A0 interrupts
	TA0CCTL1 = OUTMOD_7;
	TA0CCR0  = 255;
	TA0CCR1	 = 0;

	/* Port 1 Setup */
	P1DIR = BIT6 | LEDR;
	P1OUT = 0;
	P1SEL = BIT6;

	__enable_interrupt();

	while (1) {
		playTone(220);
		__delay_cycles(8000000);

		playTone(440);
		__delay_cycles(8000000);

		playTone(680);
		__delay_cycles(8000000);

	}
}

