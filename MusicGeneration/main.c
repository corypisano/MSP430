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
#define INPUT BIT3
#define LEDR BIT0
#define LEDG BIT6
#define MESSAGE_LENGTH 5

void playTone(unsigned int frequency, unsigned int duration) {

	TA0CCR0 = (4000000/frequency) - 1;
	TA0CCR1 = 4;

	// switch to duration using Timer
	__delay_cycles(4000000);

	// TA0CCR0 = 0; // turn note off?
}

void stopTone() {
	TA0CCR0 = 0;
	TA0CCR1 = 0;
}

void MODE_1() {
	/* Play A4 */
	playTone(440, 16000); // A4  = 440Hz
}

void MODE_2() {
	/* Play A major! */
	playTone(440, 16000); // A4  = 440Hz
	playTone(494, 16000); // B4  = 494Hz
	playTone(554, 16000); // C5# = 554Hz
	playTone(587, 16000); // D5  = 587Hz
	playTone(659, 16000); // E5  = 659Hz
	playTone(740, 16000); // F5# = 740Hz
	playTone(831, 16000); // G5# = 831Hz
}

void MODE_3() {
	/* Play a song! */
	playTone(440, 16000); // A4  = 440Hz
	playTone(494, 16000); // B4  = 494Hz
}

/* Timer A timeout interrupt */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA0 (void) {
}


void main(void) {

	WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog timer

	/* Clock Setup - 16MHz */
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL  = CALDCO_16MHZ;

	/* TIMER A0 SETUP */
	TA0CTL	 = MC_1 | ID_2 | TASSEL_2;  // divide by 4
	//TA0CCTL0 = CCIE;                  // Enable Timer A0 interrupts
	TA0CCTL1 = OUTMOD_7;
	TA0CCR0  = 0;
	TA0CCR1	 = 0;

	/* Port 1 Setup */
	P1DIR = BIT6;
	P1OUT = 0;
	P1SEL = BIT6;

	while (1) {

		/* Play A major! */
		playTone(440, 16000); // A4  = 440Hz
		playTone(494, 16000); // B4  = 494Hz
		playTone(554, 16000); // C5# = 554Hz
		playTone(587, 16000); // D5  = 587Hz
		playTone(659, 16000); // E5  = 659Hz
		playTone(740, 16000); // F5# = 740Hz
		playTone(831, 16000); // G5# = 831Hz

		stopTone();
		__delay_cycles(4000000);
	}
}



