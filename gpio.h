/*
 * gpio.h
 */

#ifndef __GPIO_H
#define __GPIO_H

#define GPIO_0		PORTAbits.RA0
#define GPIO_1		PORTAbits.RA1
#define GPIO_2		PORTBbits.RB0
#define GPIO_3		PORTBbits.RB1
#define GPIO_4		PORTBbits.RB2
#define GPIO_5		PORTBbits.RB3

#define GPIO_6		PORTAbits.RA2
#define GPIO_7		PORTAbits.RA3
#define GPIO_8		PORTBbits.RB4
#define GPIO_9		PORTBbits.RB13
#define GPIO_10		PORTBbits.RB14
#define GPIO_11		PORTBbits.RB15


#define GPIO_0_DIR         TRISAbits.TRISA0
#define GPIO_1_DIR         TRISAbits.TRISA1
#define GPIO_2_DIR         TRISBbits.TRISB0
#define GPIO_3_DIR         TRISBbits.TRISB1
#define GPIO_4_DIR         TRISBbits.TRISB2
#define GPIO_5_DIR         TRISBbits.TRISB3

#define GPIO_6_DIR	TRISAbits.TRISA2
#define GPIO_7_DIR	TRISAbits.TRISA3
#define GPIO_8_DIR	TRISBbits.TRISB4
#define GPIO_9_DIR	TRISBbits.TRISB13
#define GPIO_10_DIR	TRISBbits.TRISB14
#define GPIO_11_DIR	TRISBbits.TRISB15


#endif
