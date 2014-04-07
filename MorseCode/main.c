/*
--------------------------------------------
Cory Pisano
April 7, 2014
ECE:435   - Embedded Systems Programming
Project 1 - Morse Code
--------------------------------------------
Assignment Instructions:
Interpret a morse code input of 5 presses as a number from 1 to 9 and flash the green LED
on the Launchpad that number of times. Use TimerA to count the duration of each button press.
When a bad input is received, flash the red LED.

Counting Button Presses:
Count the duration of five buttons presses on Pin1.3. Find the longest duration and call
that a "dash". Label any button presses less than half the longest duration "dots" and any
longer than half the duration "dashes". Interpret this as a digit using the international
Morse Code standard. Notice that five presses of the same duration will be the digit 5.
While you are flashing the green LED you should ignore additional button presses, but once
that flashing completes you should be ready for the next sequence of five button presses.

Bad Inputs:
There are two bad input cases. The first is when five button presses are received that do not
correspond to a digit from 1 to 9. The second is when a sequence is started, but then 2 seconds
pass without button motion (the button stays down or up).

Power Consumption:
The power consumption of your board should be low when the LED is not on; stay in LPM3 whenever
possible.
--------------------------------------------
 * #        Morse Code
 * 0        – – – – –
 * 1        · – – – –
 * 2        · · – – –
 * 3        · · · – –
 * 4        · · · · –
 * 5        · · · · ·
 * 6        – · · · ·
 * 7        – – · · ·
 * 8        – – – · ·
 * 9        – – – – ·
--------------------------------------------
 */

#include <msp430.h>


/* globals */
#define INPUT BIT3
#define LEDR BIT0
#define LEDG BIT6
#define MESSAGE_LENGTH 5

unsigned short count = 0;			// number of times button has been pressed
unsigned short isButtonPressed = 0;		// boolean to check if button is High or Low
unsigned int pressTimes[MESSAGE_LENGTH];	// stores length of button presses

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

	/* Morse Code to # */
	if (pressTimes[0] == 0) {
		if (pressTimes[1] == 1 && pressTimes[2] == 1 && pressTimes[3] == 1 && pressTimes[4] == 1) {
			return 1;	// 1    .----
		} else if (pressTimes[1] == 0 && pressTimes[2] == 1 && pressTimes[3] == 1 && pressTimes[4] == 1) {
			return 2;	// 2    ..---
		} else if (pressTimes[1] == 0 && pressTimes[2] == 0 && pressTimes[3] == 1 && pressTimes[4] == 1) {
			return 3;	// 3    ...--
		} else if (pressTimes[1] == 0 && pressTimes[2] == 0 && pressTimes[3] == 0 && pressTimes[4] == 1) {
			return 4;	// 4    ....-
		} else {
			return 0;	// error - flash RED
		}
	} else {
		if (pressTimes[1] == 1 && pressTimes[2] == 1 && pressTimes[3] == 1 && pressTimes[4] == 1) {
			return 5;	// *** 5    ..... but implemented as -----
		} else if (pressTimes[1] == 0 && pressTimes[2] == 0 && pressTimes[3] == 0 && pressTimes[4] == 0) {
			return 6;	// 6    -....
		} else if (pressTimes[1] == 1 && pressTimes[2] == 0 && pressTimes[3] == 0 && pressTimes[4] == 0){
			return 7;	// 7    --...
		} else if (pressTimes[1] == 1 && pressTimes[2] == 1 && pressTimes[3] == 0 && pressTimes[4] == 0){
			return 8;	// 8    ---..
		} else if (pressTimes[1] == 1 && pressTimes[2] == 1 && pressTimes[3] == 1 && pressTimes[4] == 0){
			return 9;	// 9    ----.
		} else {
			return 0;	// error - flash RED
		}
	}
}

void flashLED(unsigned short n) {
	P1IE &= ~INPUT;	// ignore button presses while flashing 

	count = 0;

	/* ERROR !  FLASH LEDR 3 times */
	if (n == 0) {
		int i;
		for (i = 0; i < 3; i++) {
			P1OUT |= LEDR;
			__delay_cycles(200000);
			P1OUT &= ~LEDR;
			__delay_cycles(200000);
			TA0CTL |= TACLR;	// reset TAR,
		}
	} else {
		/* Valid message: Display on LEDG */
		int i;
		for (i = 0; i < n; i++) {
			P1OUT |= LEDG;
			__delay_cycles(200000);
			P1OUT &= ~LEDG;
			__delay_cycles(200000);
			TA0CTL |= TACLR;	// reset TAR,
		}
	}

	P1IE |= INPUT; // turn interrupts back on to allow next message
}

/* Push Button Interrupt */
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {

	P1IE &= ~INPUT;     // dont allow additional interrupts
	P1IES ^= INPUT;     // falling/rising edge switch
	isButtonPressed = !isButtonPressed; // button is in next state

	/* ignore button presses that are too quick */
	if (TA0R < 128) {
		P1IE |= INPUT;      	// turn button interrupts back on
		P1IFG &= ~(INPUT);  	// clear push button interrupt
		return;
	}

	/* ON BUTTON PRESS */
	if (isButtonPressed) {
		TA0CTL |= TACLR;	// reset TAR,
	}

	/* ON BUTTON RELEASE */
	else {
		pressTimes[count++] = TA0R;
		TA0CTL |= TACLR; 	// reset TAR,
	}

	if (count > 5){
		flashLED(0);	// ERROR: too many presses
	}

	P1IE |= INPUT;		// re-enable button interrupts
	P1IFG &= ~(INPUT);	// clear push button interrupt
}

/* Timer A timeout interrupt */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA0 (void) {

	/* no button presses have occured yet */
	if (count == 0) {
		TA0CTL |= TACLR;	// reset TAR
	}
	/* display message */
	else if (count == MESSAGE_LENGTH) {
		unsigned short message = convertToMessage();
		flashLED(message);
		count = 0;
	}
	/* display error */
	else {
		flashLED(0);
		count = 0;
	}
}

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;   // Disable watchdog timer
	BCSCTL3 |= XCAP_3;          // use external clock

	/* INITIALIZE PORT 1 */
	P1DIR = 0xFF;
	P1OUT = 0x00;

	/* LED SETUP */
	P1DIR |= LEDR + LEDG;

	/* PUSH BUTTON SETUP */
	P1DIR &= ~INPUT;    // Port 1 P1.3 (push button) as INPUT
	P1REN |= INPUT;     // Enable resistor
	P1OUT |= INPUT;     // Resistor is PULL UP
	P1IE  |= INPUT;     // Enables P1.3 (push button) to generate interrupt
	P1IES |= INPUT;     // Interrupt on rising edge (switch edge in ISR)
	P1IFG &= ~INPUT;    // clear the interrupt flag

	/* TIMER A0 SETUP */
	TA0CCTL0 = CCIE;                  // Enable Timer A0 interrupts
	TA0CTL = TASSEL_1 + MC_1 + ID_3;  // mode 1, source ACLK, div by 8
	TA0CCR0 = 8191;                   // Count limit - 2 second timeout

	_BIS_SR(LPM3_bits + GIE);         // enable interrupts

	while (1) {
	}
}
