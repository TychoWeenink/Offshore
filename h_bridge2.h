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
#define PORT_RPWMY	PORTL
#define PIN_RPWMY	PL5
#define DDR_RPWMY	DDRL

#define PORT_LPWMY	PORTL
#define PIN_LPWMY	PL4
#define DDR_LPWMY	DDRL

void init_h_bridgeY(void);
void h_bridge_set_percentageY(signed char percentageY);

#endif /* _H_BRIDGE_H_ */
