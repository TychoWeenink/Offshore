#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 16000000ul
#endif
#include <util/delay.h>
#include "h_bridge.h"
#include "h_bridge2.h"
#include "h_bridgeZ.h"
#define SDI         PH5
#define Shift_Clk   PH4
#define Latch_Clk   PG5

#define DEBOUNCE _delay_ms(10);


int init(void)
{
    init_h_bridge();
    init_h_bridgeY();
    init_h_bridgeZ();
    init_leds();
    init_knop();

    DDRH |= (1<<SDI);
    DDRH |= (1<<Shift_Clk);
    DDRG |= (1<<Latch_Clk);

	PORTH &= ~(1<<Shift_Clk);
    PORTG &= ~(1<<Latch_Clk);
}

void init_leds(void)
{
    DDRB = (1<<PB7);
    PORTB |= (1<<PB7);

    // leds x,y
    DDRJ = (1<<PJ1) | (1<<PJ0); //pin D14, D15
}

void init_output(void)
{
//    // h brug x
//    DDRE |= (1<<PE3); // pin D5 LPWM
//    DDRH |= (1<<PH3); // pin D6 RPWM
//
//    // h brug y
//    DDRL |= (1<<PL5); // pin D46 RPWM
//    DDRL |= (1<<PL4); // pin D45 LPWM
//
//    // h brug z
//    DDRL  |= (1<<PL3); // pin D46 RPWM
//    DDRH  |= (1<<PH6); // pin D9  LPWM
//
    DDRD |= (1<<PD0); // pin D21 electromagneet
}

void init_knop(void)
{
    // bedieningspaneel
    DDRF &= ~(1<<PF1); //knop A1
    DDRF &= ~(1<<PF2); //knop A2
    DDRF &= ~(1<<PF3); //knop A3

    // noodstop
    DDRF &= ~(1<<PK2); //noodstop PIN A10

    // encoder
    DDRK &= ~(1<<PK0); // A8 encoder A
    DDRK &= ~(1<<PK1); // A9 encoder B

    // eindschakelaren
    DDRK &= ~(1<<PK3); // A11 X as (0)
    DDRK &= ~(1<<PK4); // A12 Y as (0)
    DDRK &= ~(1<<PK7); // A15 Z as boven
    DDRB &= ~(1<<PB2); // D51 Z as Beneden

    DDRF &= ~(1<<PF7); // A7 electromagneet (tonnetje)

    DDRB &= ~(1<<PB1); // D52 X as eind
    DDRB &= ~(1<<PB0); // D53 Y as eind

    // tellers
    DDRK &= ~(1<<PK5); // A13 X as
    DDRK &= ~(1<<PK6); // A14 Y as
}

void send_data(char data)
{
	// Herhaal voor alle bits in een char
	int i;
    for(i = 0; i < 8; i++)
	{
	    if (data & (1<< (7-i)))
        {
            PORTH |= (1<<SDI);
        }
        else
        {
            PORTH &= ~(1<<SDI);
        }
        PORTH |= (1<<Shift_Clk);
        PORTH &= ~(1<<Shift_Clk);
		// Bepaal de waarde van de bit die je naar het schuifregister
		// wil sturen
		// Maak de juiste pin hoog of laag op basis van de bepaalde waarde
		// van het bit
		// Toggle shiftclk (hoeveel tijd moest het signaal minimaal hoog zijn?)
	}
}

void display(char data, char disp)
{
	send_data(data);
	send_data(disp);
	PORTG |= (1<< Latch_Clk);
    PORTG &= ~(1<< Latch_Clk);
	// Toggle latchclk (hoeveel tijd moest het signaal minimaal hoog zijn?)
}

int main(void)
{
    int arrayX[]={0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x83, 0xF8, 0x80, 0x90};
    int arrayY[]={~(0x77), ~(0x7C), ~(0x39), ~(0x5E), ~(0x79), ~(0x71)};
    int arrayXrot[]={4, 7, 10, 13, 16, 18, 21, 24, 27, 30};
    int arrayYrot[]={8, 16, 24, 32, 40, 48};
    init();
    static int tellerxco = 0;
    static int telleryco = 0;
    int XOM = 0;
    int YOM = 0;
    int toestand = 0;
    int knop_ingedrukt = 1;
    int Eind = 0;
    int ton = 0;

    while(1)
    {
        if((PINF &(1<<PF3)) ==0) // reset
        {
          DEBOUNCE;
          if((PINF &(1<<PF3)) !=0)
                {
                    DEBOUNCE;
                    h_bridge_set_percentage(70);
                    h_bridge_set_percentageY(70);
                    h_bridge_set_percentageZ(-70);
                    XOM = 0;
                    YOM = 0;
                    toestand = 8; // vanuit alle toestanden
                }
        }

       if((PINK &(1<<PK2)) !=0) // noodstop
        {
          DEBOUNCE;
//                if((PINK &(1<<PK2)) ==0)
//                {
//                    DEBOUNCE;
                    h_bridge_set_percentage(0);
                    h_bridge_set_percentageY(0);
                    h_bridge_set_percentageZ(0);
                    toestand = 9; // vanuit alle toestanden
//                }
        }

        // einde to noodstand
        if((PINB &(1<<PB1)) !=0) // einde X as (te ver)
        {
            DEBOUNCE;
            h_bridge_set_percentage(70);
            Eind = 1;
            toestand = 15;
        }
        if((PINB &(1<<PB0)) !=0) // einde Y as (te ver)
        {
            DEBOUNCE;
            h_bridge_set_percentageY(70);
            Eind = 1;
            toestand = 15;
        }

        // voorkomen van te grote waarde na 9 terug naar 0 en na F terug naar A (Y=0--6)
        if(tellerxco < 0)
        {
            tellerxco = 9;
        }
        if(tellerxco > 9)
        {
            tellerxco = 0;
        }
        if(telleryco < 0)
        {
            telleryco = 5;
        }
        if(telleryco > 5)
        {
            telleryco = 0;
        }

        switch (toestand)
        {
    case 0:
        display(0xC0, 0x01);    // o
        display(~(0x54), 0x02); // n
        PORTD = (1<<PD0);
        if((PINF &(1<<PF1)) ==0)
                    {
                        DEBOUNCE;
                        if((PINF &(1<<PF1)) !=0)
                        {
                            DEBOUNCE;
                            PORTJ = (1<<PJ1); // LED X (as) aan
                            PORTB &= ~(1<<PB7); //LED 4 aan
                            toestand = 1;
                            h_bridge_set_percentage(0);
                            h_bridge_set_percentageY(0);
                        }
                    }
        break;

        case 1:                                 // invoer xas coordinaat
           display(0xF9, 0x01); // 1 eerste
	       display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	       display(arrayY[telleryco], 0x08);     // A (y) A-F


           if((PINK &(1<<PK0)) !=0) // X+ coordinaat
            {
                DEBOUNCE;
                tellerxco++;
            }

           if((PINK &(1<<PK1)) !=0) // x- coordinaat
            {
                DEBOUNCE;
                tellerxco--;
            }
            if((PINF &(1<<PF1)) ==0)// selectie ok door naar yas coordinaat
            {
                DEBOUNCE;

                if((PINF &(1<<PF1)) !=0)
                {
                    DEBOUNCE;
                    PORTJ |= (1<<PJ1); // X as LED uit
                    PORTJ = (1<<PJ0);  // Y as LED aan
                    toestand = 2;
                }
            }

            break;
        case 2:                             //invoer yas coordinaat
            display(0xA4, 0x01); // 2 eerste
	        display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            if((PINK &(1<<PK0)) !=0)// y+ coordinaat
            {
              DEBOUNCE;
              telleryco++;
            }
            if((PINK &(1<<PK1)) !=0)// y- coordinaat
            {
              DEBOUNCE;
              telleryco--;
            }

            if((PINF &(1<<PF2)) ==0)// cancel terug naar x + of -
            {
              DEBOUNCE;
              PORTJ = (1<<PJ1);
              toestand = 1;
            }

            if((PINF &(1<<PF7)) !=0) // ton aanwezig
            {
                if((PINF &(1<<PF1)) ==0) // ok door naar bewegen 3
                {
                    DEBOUNCE;

                    if((PINF &(1<<PF1)) !=0)
                    {
                        DEBOUNCE;
                        if((PINF &(PF7)) != 0)
                        {
                            PORTJ |= (1<<PJ1) + (1<<PJ0); // Beide X en Y LEDS aan
                            h_bridge_set_percentage(-70);
                            h_bridge_set_percentageY(-70);
                            toestand = 3;
                        }
                    }
                }
            }
             if((PINF &(1<<PF7)) ==0) // geen ton aanwezig
            {
                if((PINF &(1<<PF1)) ==0) // ok door naar ton oppakken 4
                {
                    DEBOUNCE;

                    if((PINF &(1<<PF1)) !=0)
                    {
                        DEBOUNCE;
                        if((PINF &(PF7)) != 0)
                        {
                            PORTJ |= (1<<PJ1) + (1<<PJ0); // Beide X en Y LEDS aan
                            h_bridge_set_percentageZ(100);
                            toestand = 4;
                        }
                    }
                }
            }


            break;
        case 3:                              // x en y as
            display(0xB0, 0x01); // 3 eerste
	        display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

	        if((PINK &(1<<PK5)) != 0)
            {
                DEBOUNCE;
                if((PINK &(1<<PK5)) ==0)
                {
                    XOM++; // schakelaar voor X-as optellen
                }
            }

            if(XOM == (arrayXrot[tellerxco])) // Optelling gelijk aan arrayX waarde stop
            {
                h_bridge_set_percentage(0);
//                XOM = 0;
            }

            if((PINK &(1<<PK6)) != 0) // knop release
            {
                DEBOUNCE;
                if((PINK &(1<<PK6)) ==0)
                    {
                        DEBOUNCE;
                        YOM++; // schakelaar voor Y-as optellen
                    }
            }

            if(YOM == (arrayYrot[telleryco])) // Optelling gelijk aan arrayY waarde stop
            {
                h_bridge_set_percentageY(0);
//                YOM = 0;
            }

            if( XOM == (arrayXrot[tellerxco]) && YOM == (arrayYrot[telleryco])) // als beide assen op plek zijn z as omlaag
            {
                h_bridge_set_percentageZ(70);
                toestand = 6;
            }

            if((PINF &(PF1)) ==0)
            {
               DEBOUNCE;
               if((PINF &(PF1)) != 0)
               {
                h_bridge_set_percentageZ(70);
                toestand = 6;
               }
            }
            break;

        case 4:                             // z as omlaag na invoer
            display(0x99, 0x01); // 4 eerste
            display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            if((PINB &(1<<PB2)) != 0) // als z as omlaag is stop en door naar 6
            {
                DEBOUNCE;
                h_bridge_set_percentageZ(0);
            }

            if((PINB &(1<<PB2)) && (PINF &(1<<PF7)) != 0)
            {
                DEBOUNCE;
                h_bridge_set_percentageZ(-100);
                toestand = 5;
            }

            if((PINB &(1<<PB2)) != 0)
            {
                DEBOUNCE;
                if((PINK &(1<<PK2)) == 0)
                {
                    DEBOUNCE;
                    if((PINK &(1<<PK2)) !=0)
                    {
                        DEBOUNCE;
                            h_bridge_set_percentageZ(-70);
                            toestand = 13;
                    }
                }
            }

            break;

        case 5:                             // z as omhoog met ton
            display(0x92, 0x01); // 5 eerste
            display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            if((PINK &(1<<PK7)) != 0) // als z as omlaag is stop en door naar 6
            {
                DEBOUNCE;
                h_bridge_set_percentageZ(0);
                h_bridge_set_percentage(-100);
                h_bridge_set_percentageY(-70);
                toestand = 3;
            }

            break;

        case 6:                              // ton neerzetten
            display(0x83, 0x01); // 6 eerste
            display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            if((PINB &(1<<PB2)) !=0) // limit switch zas
             {
               DEBOUNCE;
               h_bridge_set_percentageZ(0); // z as stop
               PORTD |= (1<<PD0);             // electromagneet uit
               toestand = 7;
             }

            break;

        case 7:                             // omhoog na neerzetten
            display(0xF8, 0x01); // 7 eerste
            display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            PORTD = (1<<PD0);                // electromagneet uit
            h_bridge_set_percentageZ(-100); // z as omhoog
            if ((PINK &(1<<PK7)) ==0)       // z as omhoog gegaan door naar 13
            {
                DEBOUNCE;
                PORTD ^= (1<<PD0);           // electromagneet aan
                toestand = 14;
            }
            break;

        case 14:                                         // z as stop omhoog terug naar huis
                        display(0x92, 0x01); // 5 eerste
            display(arrayX[tellerxco], 0x04);     // 0 (x) 0-9
	        display(arrayY[telleryco], 0x08);     // A (y) A-F

            if((PINK &(1<<PK7)) != 0) // als z as omhoog is stop en door naar 8
            {
                DEBOUNCE;
                h_bridge_set_percentageZ(0);
                h_bridge_set_percentage(100); // x as naar startpunt
                h_bridge_set_percentageY(70); // y as naar startpunt
                toestand = 8;
            }

            break;

        case 8:     // resetten
            display(0x80, 0x01); // 8 eerste

             // xas - tot switch
             if((PINK &(1<<PK3)) !=0)  // limit switch xas
             {
               DEBOUNCE;
               h_bridge_set_percentage(0);

               display(0xC0, 0x04);        // X display = 0
             }

             if((PINK &(1<<PK4)) !=0) // limit switch yas
             {
               DEBOUNCE;
               h_bridge_set_percentageY(0);

               display(0xC0, 0x08);        // Y display = 0
             }

             if((PINK &(1<<PK7)) !=0) // limit switch zas
             {
               DEBOUNCE;
               h_bridge_set_percentageZ(0);

               display(0xC0, 0x02);        // vak 2 display = 0
             }

             if((PINK &(1<<PK7)) != 0 && (PINK &(1<<PK4)) != 0 && (PINK &(1<<PK3)) !=0)
               {
                   DEBOUNCE;
                if((PINF &(1<<PF1)) ==0) // knop OK terug naar case 1
                {
                  DEBOUNCE;

                    if((PINF &(1<<PF1)) !=0)
                    {
                        DEBOUNCE;
                        PORTJ = (1<<PJ1);
                        tellerxco = 0;
                        telleryco = 0;
                        XOM = 0;
                        YOM = 0;
                        toestand = 1;
                    }
                }
               }
            break;
        case 9:     // noodstop
            display(0x90, 0x01); // 9 eerste
            h_bridge_set_percentage(0);
            h_bridge_set_percentageY(0);
            h_bridge_set_percentageZ(0);

            if (Eind > 0)
            {
                if(PINF &(1<<PF3)==0)
                   {
                       DEBOUNCE;
                       if((PINF &(1<<PF3)) !=0)
                        {
                            DEBOUNCE;
                            Eind = 0;
                            h_bridge_set_percentage(70);
                            h_bridge_set_percentageY(70);
                            toestand = 10;
                        }
                   }
            }
            else
            {
                if(PINF &(1<<PF3)==0)
                   {
                       DEBOUNCE;
                       if((PINF &(1<<PF3)) !=0)
                        {
                            DEBOUNCE;
                            h_bridge_set_percentage(70);
                            h_bridge_set_percentageY(70);
                            h_bridge_set_percentageZ(-70);
                            toestand = 8;
                        }
                   }
            }
            if((PINF &(1<<PF1)) == 0)
            {
                DEBOUNCE;
                if((PINF&(1<<PF1))!=0)
                {
                    if(XOM == (arrayXrot[tellerxco])) // Optelling gelijk aan arrayX waarde stop
                        {
                            h_bridge_set_percentage(0);
                            XOM = 0;
                        }
                    else
                    {
                        h_bridge_set_percentage(-70);
                    }
                    if(YOM == (arrayYrot[telleryco])) // Optelling gelijk aan arrayY waarde stop
                        {
                            h_bridge_set_percentageY(0);
                            YOM = 0;
                        }
                    else
                    {
                        h_bridge_set_percentageY(-70);
                    }
                    toestand = 3;
                }
            }

        break;

        // te ver noodprocedure
        case 10:                         // x as en y as naar 0 check voor ton zo niet naar invoer wel ton neerzetten
            display(0xF9, 0x01);    // 1
            display(~(0x54), 0x02); // 0

            // x en y naar 0
            if((PINK &(1<<PK3)) !=0)  // limit switch xas
             {
               DEBOUNCE;
               h_bridge_set_percentage(0);

               display(0xC9, 0x04);        // X display = 0
             }

             if((PINK &(1<<PK4)) !=0) // limit switch yas
             {
               DEBOUNCE;
               h_bridge_set_percentageY(0);

               display(0xC9, 0x08);        // Y display = 0
             }
             if((PINK &(1<<PK4)) != 0 && (PINK &(1<<PK3)) !=0)
             {
                DEBOUNCE;
                 if((PINF &(1<<PF7)) != 0) // check ton ja naar 11 nee naar 1
                 {
                   DEBOUNCE;
                   h_bridge_set_percentageZ(-100);
                   toestand = 11;
                    Eind = 0;
                 }
                 else
                 {
                     PORTJ = (1<<PJ1);
                     Eind = 0;
                     XOM = 0;
                     YOM = 0;
                     toestand = 1;
                 }
             }

        break;

        case 11:                    // z as omlaag zo ja stop door naar 12
            if((PINB &(1<<PB2)) !=0) // limit switch zas
             {
               DEBOUNCE;
               h_bridge_set_percentageZ(0);
               PORTD = (1<<PD0);                // electromagneet uit
               toestand = 12;
             }
        break;

        case 12:                    // ton loslaten en omhoog
            h_bridge_set_percentageZ(-100); // z as omhoog
            if ((PINB &(1<<PB2)) ==0)       // z as omhoog gegaan door naar 13
            {
                DEBOUNCE;
                PORTD |= (1<<PD0);           // electromagneet aan
                toestand = 13;
            }
        break;

        case 13:
         if((PINK &(1<<PK7)) !=0)   // z as omhoog zo ja stop door naar case 1
         {
             DEBOUNCE;
             h_bridge_set_percentageZ(0);
             Eind = 0;
             tellerxco = 0;
             telleryco = 0;
             PORTJ = (1<<PJ1);
             toestand = 1;
         }
         break;

        case 15:
            {
                if((PINB &(1<<PB1)) ==0) // einde X as (te ver)
                    {
                        DEBOUNCE;
                        h_bridge_set_percentage(0);
                        toestand = 9;
                    }
                if((PINB &(1<<PB0)) ==0) // einde Y as (te ver)
                    {
                        DEBOUNCE;
                        h_bridge_set_percentageY(0);
                        toestand = 9;
                    }

            }

        break;
        }
    }
}
