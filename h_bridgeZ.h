/*
 * h_bridge.h - XvR 2020
 */

#ifndef _H_BRIDGE_H_
#define _H_BRIDGE_H_

// These pins are available on the shield via the header:
//
//		Mega	Uno
// digital 5	PE3	PD5
// digital 6	PH3	PD6
// digital 9	PH6	PB1
// analog 5	PF5	PC5
// digital 46   PL3
// digital 45   PL4
// digtial 44   PL5

// The settings below are for the Mega, modify
// in case you want to use other pins
#define PORT_RPWMZ	PORTH
#define PIN_RPWMZ	PH6     //D9
#define DDR_RPWMZ	DDRH

#define PORT_LPWMZ	PORTL
#define PIN_LPWMZ	PL3     //D46
#define DDR_LPWMZ	DDRL

void init_h_bridgeZ(void);
void h_bridge_set_percentageZ(signed char percentageZ);

#endif /* _H_BRIDGE_H_ */
