/*
 * h_bridge.c - XvR 2020
 *
 * Use 8-bit timer. Uses interrupts in order to be able
 * to use the pins on the multifunction shield
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "h_bridgeZ.h"

ISR(TIMER2_OVF_vect)
{
	if (OCR2A == 0 && OCR2B == 0)
	{
		PORT_RPWMZ &= ~(1<<PIN_RPWMZ);
		PORT_LPWMZ &= ~(1<<PIN_LPWMZ);
	}
	else if (OCR2A != 0)
	{
		PORT_LPWMZ &= ~(1<<PIN_LPWMZ);
		PORT_RPWMZ |= (1<<PIN_RPWMZ);
	}
	else if (OCR2B != 0)
	{
		PORT_RPWMZ &= ~(1<<PIN_RPWMZ);
		PORT_LPWMZ |= (1<<PIN_LPWMZ);
	}
}

ISR(TIMER2_COMPA_vect)
{
	if (OCR2A != 255)
	{
		PORT_RPWMZ &= ~(1<<PIN_RPWMZ);
	}
}

ISR(TIMER2_COMPB_vect)
{
	if (OCR2B != 255)
	{
		PORT_LPWMZ &= ~(1<<PIN_LPWMZ);
	}
}

void init_h_bridgeZ(void)
{
	// Config pins as output
	DDR_RPWMZ |= (1<<PIN_RPWMZ);
	DDR_LPWMZ |= (1<<PIN_LPWMZ);

	// Output low
	PORT_RPWMZ &= ~(1<<PIN_RPWMZ);
	PORT_LPWMZ &= ~(1<<PIN_LPWMZ);

	// Use mode 0, clkdiv = 64
	TCCR2A = 0;
	TCCR2B = (0<<CS22) | (1<<CS21) | (1<<CS20);

	// Disable PWM output
	OCR2A = 0;
	OCR2B = 0;

	// Interrupts on OCA, OCB and OVF
	TIMSK2 = (1<<OCIE2B) | (1<<OCIE2A) | (1<<TOIE2);

	sei();
}

void h_bridge_set_percentageZ(signed char percentageZ)
{
	if (percentageZ >= -100 && percentageZ <= 100)
	{
		if (percentageZ >= 0)
		{
			// Disable LPWM, calculate RPWM
			OCR2B = 0;
			OCR2A = (255*percentageZ)/100;
		}
		else // percentage < 0
		{
			// Disable RPWM, calculate LPWM
			OCR2A = 0;
			OCR2B = (255*percentageZ)/-100;
		}
	}
}
