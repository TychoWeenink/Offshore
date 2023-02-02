/*
 * h_bridge.c - XvR 2020
 *
 * Use 8-bit timer. Uses interrupts in order to be able
 * to use the pins on the multifunction shield
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "h_bridge2.h"

ISR(TIMER3_OVF_vect)
{
	if (OCR3A == 0 && OCR3B == 0)
	{
		PORT_RPWMY &= ~(1<<PIN_RPWMY);
		PORT_LPWMY &= ~(1<<PIN_LPWMY);
	}
	else if (OCR3A != 0)
	{
		PORT_LPWMY &= ~(1<<PIN_LPWMY);
		PORT_RPWMY |= (1<<PIN_RPWMY);
	}
	else if (OCR3B != 0)
	{
		PORT_RPWMY &= ~(1<<PIN_RPWMY);
		PORT_LPWMY |= (1<<PIN_LPWMY);
	}
}

ISR(TIMER3_COMPA_vect)
{
	if (OCR3A != 65535)
	{
		PORT_RPWMY &= ~(1<<PIN_RPWMY);
	}
}

ISR(TIMER3_COMPB_vect)
{
	if (OCR3B != 65535)
	{
		PORT_LPWMY &= ~(1<<PIN_LPWMY);
	}
}

void init_h_bridgeY(void)
{
	// Config pins as output
	DDR_RPWMY |= (1<<PIN_RPWMY);
	DDR_LPWMY |= (1<<PIN_LPWMY);

	// Output low
	PORT_RPWMY &= ~(1<<PIN_RPWMY);
	PORT_LPWMY &= ~(1<<PIN_LPWMY);

	// Use mode 0, clkdiv = 64
	TCCR3A = 0;
	TCCR3B = (0<<CS32) | (0<<CS31) | (1<<CS30);

	// Disable PWM output
	OCR3A = 0;
	OCR3B = 0;

	// Interrupts on OCA, OCB and OVF
	TIMSK3 = (1<<OCIE3B) | (1<<OCIE3A) | (1<<TOIE3);

	sei();
}

void h_bridge_set_percentageY(signed char percentageY)
{
	if (percentageY >= -100 && percentageY <= 100)
	{
		if (percentageY >= 0)
		{
			// Disable LPWM, calculate RPWM
			OCR3B = 0;
			OCR3A = (65535*percentageY)/100;
		}
		else // percentage < 0
		{
			// Disable RPWM, calculate LPWM
			OCR3A = 0;
			OCR3B = (65535*percentageY)/-100;
		}
	}
}
