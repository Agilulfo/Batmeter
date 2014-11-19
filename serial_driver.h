/*
 *  serial_driver.h
 */

#ifndef __SERIAL_DRIVER_H
#define __SERIAL_DRIVER_H

#include "defines.h"

#define MAX_REPLY_SIZE   64

typedef struct {
    char reply_string[MAX_REPLY_SIZE];
    int reply_size;
    int reply_ready;
} t_reply;

extern t_reply REPLY;

#define is_reply_ready()  (REPLY.reply_ready == 1)
#define reset_reply()  { REPLY.reply_size = REPLY.reply_ready = 0 ; }

#ifdef MICROBRAIN_MILLEFORI

#define IO485_ENABLE_1() LATBbits.LATB7 = 1
#define IO485_DISABLE_1() {                     \
    while (U1STAbits.TRMT == 0);                \
    LATBbits.LATB7 = 0;                         \
    }

#else

#define IO485_ENABLE_1() LATBbits.LATB12 = 1
#define IO485_DISABLE_1() {                     \
    while (U1STAbits.TRMT == 0);                \
    LATBbits.LATB12 = 0;                         \
    }

#endif

#define TIMER_TICK_TIMEOUT 300

extern unsigned short timer_tick;

#define is_timeout()  (timer_tick > TIMER_TICK_TIMEOUT)
#define timer_reset() timer_tick = 0

// ------------------------------------------------------------

#ifdef EXTERNAL_CLOCK

#define B115200         20//???

#endif

// ------------------------------------------------------------

#ifdef INTERNAL_CLOCK

#define B115200         20

#endif

// ------------------------------------------------------------

void serial_start(int use_interrupt, int baud_rate) ;

void serial_stop() ;

void serial2_start(int use_interrupt, int baud_rate) ;

void serial2_stop() ;

void serial_output(char *uBuffer, int uBufferLength);

int serial_input(unsigned char* buffer) ;

int serial_getc(unsigned char * c) ;

int serial_getbuffer(unsigned char * buf, int size);

#endif
