#include <msp430.h> 
#include "MMC.h"
#include "hal_SPI.h"
/*
 *
 * http://blog.silabs.com/2013/02/07/how-to-interface-an-sd-card-with-fat-file-system-using-spi/
 *
 *  SDCARD
 *  {9} 1 	2 		3 		4		5 		6 		[7|8]
 *  	CS	SIMO	GND		VCC		CLK		Detect	SOMI
 *
 *  	SOMI 	: BIT1
 *  	SIMO 	: BIT2
 *  	CS 		: BIT3
 *  	CLK  	: BIT4
 *  	DETECT 	: BIT5
 */

#define LEDR BIT0
#define LEDG BIT6
const unsigned char SOMI = BIT1;
const unsigned char SIMO = BIT2;
const unsigned char SCLK = BIT4;

volatile unsigned long cardSize = 0;
volatile unsigned char status = 1;

//Set up the port 1 pins
void setPins() {
	//Port 1
	P1DIR = LEDR + LEDG;
	P1OUT = 0x00; // Output off, input resistors set to pulldown (including SOMI)
}


void set_UCS() {
	/* DCO Frequency selected */
	/* SCLK=SMCLK  P1.5  */
	/*	Maximum SPI frequency for burst mode is 6.5 MHz	*/
	//Set DCO to 12 MHz calibrated and use DCO/2 for SMCLK
	DCOCTL = CALDCO_12MHZ;		//0x7A  01111010
	//DCOx  011
	//MODx	11010

	/* External Crystal OFF, LFEXT1 is low frequency, ACLK is Divided by 1, RSELx=1110 */
	BCSCTL1 = CALBC1_12MHZ;       // Set range  0x8E  10001110

	/* DCO -> MCLK , MCLK is divided by 2, DCO -> SMCLK, SMCLK is divided by 2, Internal Resistor */
	BCSCTL2 = 0x52;  //  01010010

	/* 0.4 to 1MHz crystal , Using VLO for ACLK, Cap selected 1pF */
	BCSCTL3 = 0x20;
	IE1 &= 0xFD; /* Disable UCS interrupt */

	return;
}


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	/* Set clock and pins */
	set_UCS();
	setPins();

	/* Initialize */
	while (status != 0)	{
		status = mmcInit();
	}
	P1OUT |= LEDR;	// red LED is lit when initialized

	while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted

	P1OUT |= LEDG;	// red and green lit when ping==success

	// Read the Card Size from the CSD Register
	cardSize =  mmcReadCardSize();

	if (cardSize > 20000000) {
		P1OUT &= ~(LEDR);  // green is on for good card size read
	} else {
		P1OUT &= ~(LEDG);  // red is on for bad card size read
	}

	while (1) {
	}
	return 0;
}
