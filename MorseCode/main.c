/*
Cory Pisano
April 7, 2014
ECE:435   - Embedded Systems Programming
Project 1 - Morse Code

 ----------------------
 * #        Morse Code
 * 0 		– – – – –
 * 1    	· – – – –
 * 2  		· · – – –
 * 3 		· · · – –
 * 4      	· · · · –
 * 5      	· · · · ·
 * 6      	– · · · ·
 * 7      	– – · · ·
 * 8      	– – – · ·
 * 9      	– – – – ·
----------------------
 */

#include <msp430.h>

#define INPUT BIT3
#define LEDR BIT0
#define LEDG BIT6
#define MESSAGE_LENGTH 5

/* globals */
unsigned short count = 0;
unsigned short isButtonPressed = 0;
unsigned int pressTimes[MESSAGE_LENGTH];

unsigned short convertToMessage() {

	/* get max time */
	unsigned int maxTime = pressTimes[0];
	unsigned int i;
	for (i = 1; i < MESSAGE_LENGTH; i++) {
		if (pressTimes[i] > maxTime) {
			maxTime = pressTimes[i];
		}
	}

	unsigned int threshold = maxTime / 2;

	/* pressTimes -> message */
	for (i = 0; i < MESSAGE_LENGTH; i++) {
		if (pressTimes[i] > threshold) {
			pressTimes[i] = 1;
		}
		else {
			pressTimes[i] = 0;
		}
	}

	/* Morse Code to # */ 						// # 	Morse Code
	if (pressTimes[0] == 0) {
		if (pressTimes[1] == 1) {				// 1 	.----
			return 1;
		} else if (pressTimes[2] == 1) { 		// 2 	..---
			return 2;
		} else if (pressTimes[3] == 1) {		// 3 	...--
			return 3;
		} else if (pressTimes[4] == 1) {		// 4 	....-
			return 4;
		} else {								// 5 	.....
			return 5;
		}
	} else {
		if (pressTimes[1] == 0) { 				// 6 	-....
			return 6;
		} else if (pressTimes[2] == 0) {		// 7 	--...
			return 7;
		} else if (pressTimes[3] == 0) {		// 8 	---..
			return 8;
		} else {								// 9 	----.
			return 9;
		}
	}
}

void flashLEDG(unsigned short n) {
	/*ignore button presses while flashing */
	P1IE &= ~INPUT;

	/* flash led n times, switch to interrupt */
	int i;
	for (i = 0; i < n; i++) {
		P1OUT |= LEDG;
		__delay_cycles(250000);
		P1OUT &= ~LEDG;
		__delay_cycles(250000);
		TA0CTL |= TACLR; // reset TAR,
	}
	P1IE |= INPUT;
}

void flashLEDR(unsigned short n) {

	/*ignore button pressed while flashing */
	P1IE &= ~INPUT;

	/* flash led n times, switch to interrupt */
	int i;
	for (i = 0; i < n; i++) {
		P1OUT |= LEDR;
		__delay_cycles(250000);
		P1OUT &= ~LEDR;
		__delay_cycles(250000);
		TA0CTL |= TACLR; // reset TAR,
	}
	P1IE |= INPUT;
}

/* Push Button Interrupts */
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {

	P1IE &= ~INPUT;  	// dont allow additional interrupts
	P1IES ^= INPUT;		// falling/rising edge switch
	isButtonPressed = !isButtonPressed;

	/* DEBOUNCE - ignore button presses that are too quick */
	if (TA0R < 128) {
		P1IE |= INPUT;  	// turn button interrupts back on
		P1IFG &= ~(INPUT);	// clear push button interrupt
		return;
	}

	/* ON BUTTON PRESS */
	if (isButtonPressed) {
		TA0CTL |= TACLR; // reset TAR,
		P1OUT |= LEDR;
	}
	/* ON BUTTON RELEASE */
	else {
		pressTimes[count++] = TA0R;
		TA0CTL |= TACLR; // reset TAR,
		P1OUT &= ~LEDR;
	}

	P1IE |= INPUT;
	P1IFG &= ~(INPUT);	// clear push button interrupt
}

/* Timer A timeout interrupt
 * check if message is valid -> display green flashes
 * 	     else ERROR -> display red flashes */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA0 (void) {		// Timer0 A0 interrupt service routine

	/* no button pressed have occured yet */
	if (count == 0) {
		TA0CTL |= TACLR; // reset TAR
	}
	/* display message */
	else if (count == MESSAGE_LENGTH) {
		unsigned short message = convertToMessage();
		count = 0;
		flashLEDG(message);
		count = 0;
	}
	/* error */
	else {
		flashLEDR(2);
	}
}

void main(void) {

	WDTCTL = WDTPW + WDTHOLD; 	// Disable watchdog timer
	BCSCTL3 |= XCAP_3; 			// use external clock

	/* INITIALIZE PORT 1 */
	P1DIR = 0xFF;
	P1OUT = 0x00;

	/* LED SETUP */
	P1DIR |= LEDR + LEDG;

	/* PUSH BUTTON SETUP */
	P1DIR &= ~INPUT;    // Port 1 P1.3 (push button) as INPUT
	P1REN |= INPUT;     // Enable resistor
	P1OUT |= INPUT;		// Resistor is PULL UP
	P1IE  |= INPUT; 	// Enables P1.3 (push button) to generate interrupt
	P1IES |= INPUT;	    // Interrupt on rising edge (switch edge in ISR)
	P1IFG &= ~INPUT;	// clear the interrupt flag

	/* TIMER A0 SETUP */
	TA0CCTL0 = CCIE;                  // Enable Timer A0 interrupts
	TA0CTL = TASSEL_1 + MC_1 + ID_3;  // mode 1, source ACLK, div by 8
	TA0CCR0 = 8191;                   // Count limit - 2 second timeout

	_BIS_SR(LPM3_bits + GIE);		  // enable interrupts

	while (1) {
	}
}
