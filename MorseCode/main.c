/*
 Project 1
 Due April 7

 1. fire interrupt on button press (active low)
 2. count how long button was pressed, record in array
 3. record up to 5 presses
 4. low power mode

 */

#include <msp430.h>

#define INPUT BIT3
#define LEDR BIT0
#define LEDG BIT6

/* globals */
int count = 0;
int pressTimes[5];
int isButtonPressed = 0;

void flashLEDG(int n) {

	/*ignore button presses while flashing */
	P1IE &= ~INPUT;

	/* error */
	if (n < 0) {
		P1OUT |= LEDR;
		return;
	}

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

void flashLEDR(int n) {

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
	if (isButtonPressed) {
		TA0CTL |= TACLR; // reset TAR,
		P1OUT |= LEDR;

	}
	else {
		int time_end = TAR;
		pressTimes[count++] = time_end;
		TA0CTL |= TACLR; // reset TAR,
		P1OUT &= ~LEDR;
	}

	P1OUT ^= LEDG;
	P1IE |= INPUT;  	// dont allow additional interrupts
	P1IFG &= ~(INPUT);	// clear push button interrupt
}

/* Timer A timeout interrupt
 * check if message is valid -> display green flashes
 * 	     else ERROR -> display red flashes */
#pragma vector=TIMER0_A0_VECTOR
   __interrupt void TimerA0 (void) {		// Timer0 A0 interrupt service routine
	   /* if count != 5  {
	    * 	ERROR
	    * }
	    * else
	    * 	displaymessage();
	    */
	   /* no button pressed have occured yet */
	   if (count == 0) {
		  TA0CTL |= TACLR; // reset TAR
	   }
	   else if (count == 5) {
		  flashLEDG(2);
		  count = 0;
	   }
	   else {
		   flashLEDR(2);
	   }
}

/* use timer A1 to debounce button, disable P1IE on press,
 * wait for x milliseconds to re-enable */
#pragma vector=TIMER0_A1_VECTOR
   __interrupt void TimerA1 (void) {

}


/*
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
 */

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
