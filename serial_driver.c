/*
 * serial_driver.c
 */

#include "defines.h"

#include <p33FJ128MC802.h>
#include <libpic30.h>
#include <stdio.h>
#include "serial_driver.h"
#include "led.h"



/*
 * TIMEOUT definition:
 * with internal clock (FCY = 77385000 / 2) and a prescaler of 256,
 * timer units are 6.62 microseconds
 */


#define TIMEOUT 65444u  // about 443 milliseconds

#define ONE_MS_TICK  151

unsigned short timer_tick;

t_reply REPLY;

void timer_start()
{
    T1CONbits.TON = 0; //stop
    TMR1 = 0;
    PR1 = ONE_MS_TICK; //reset timer value
    timer_tick = 0;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;// enable Timer1 interrupt
    T1CONbits.TON = 1; //start
}

void timer_stop()
{
    T1CONbits.TON = 0;
    IFS0bits.T1IF = 0;// clear interrupt flag
    IEC0bits.T1IE = 0;// disable Timer1 interrupt
}



/* DRIVER INTERFACE */


void serial_start(int use_interrupt, int baud_rate)
{
    //INIZIALIZZAZIONE SERIALE

    REPLY.reply_size = 0;
    REPLY.reply_ready = 0;

#ifdef MICROBRAIN_MILLEFORI
    //set in open drain configuration
    ODCBbits.ODCB11 = 1; //Tx
    ODCBbits.ODCB7 = 1; //Enable_485

    //set tx_pin as output
    TRISBbits.TRISB11 = PORT_OUT;
    //set rx_pin as input
    TRISBbits.TRISB10 = PORT_IN;

    //set Enable_485 as output
    TRISBbits.TRISB7 = PORT_OUT;

    //Assign rx,tx to the corresponding pins
    RPINR18bits.U1RXR = 0b01010; //Rp10
    RPOR5bits.RP11R = 0b00011; //U1TX -> Rp11
#else
    // TX = RB10
    // RX = RB11
    // ENABLE= RB12
    //set in open drain configuration
    ODCBbits.ODCB10 = 1; //Tx
    ODCBbits.ODCB12 = 1; //Enable_485

    //set tx_pin as output
    TRISBbits.TRISB10 = PORT_OUT;
    //set rx_pin as input
    TRISBbits.TRISB11 = PORT_IN;

    //set Enable_485 as output
    TRISBbits.TRISB12 = PORT_OUT;

    //Assign rx,tx to the corresponding pins
    RPINR18bits.U1RXR = 0b01011; //Rp11
    RPOR5bits.RP10R = 0b00011; //U1TX -> Rp10
#endif

    //SET SERIAL
    U1MODEbits.STSEL = 0; // 1-stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud Disabled
    U1MODEbits.BRGH = 0; // Low Speed mode
    U1BRG = baud_rate; // BAUD Rate Setting for 115200

    U1MODEbits.UEN = 0b00;
    U1STAbits.URXISEL = 0b00;

    U1STAbits.UTXISEL0 = 0;
    U1STAbits.UTXISEL1 = 0;

    //#ifndef DEBUG_232
    IPC2bits.U1RXIP = 0b111; // set hightest level priority
    IFS0bits.U1RXIF = 0; // clear interrupt flag
    IEC0bits.U1RXIE = use_interrupt; // set RX interrupt

    IEC0bits.U1TXIE = 0; // no TX interrupt

    //#endif

    //IPC2bits.U1RXIP = 2;
    U1MODEbits.UARTEN = 1; // Enable UART

    U1STAbits.UTXEN = 1; // Enable UART Tx

    LATBbits.LATB7 = 0; //Scrittura su 485 disabilitata
    LATBbits.LATB11 = 1; //Tx line set to 1

    //initialize timer
    T1CONbits.TON = 0; //timer off
    T1CONbits.TCS = 0; //internal clock
    T1CONbits.TGATE = 0; //mod gated off
    T1CONbits.TSYNC = 0; // mod sync off

    /*prescaler:
     *11 = 1:256 prescaler value
     *10 = 1:64 prescaler value
     *01 = 1:8 prescaler value
     *00 = 1:1 prescaler value
     */
    T1CONbits.TCKPS = 0b11;

    IPC0bits.T1IP = 0x02; //set Timer1 priorityLevel
    IFS0bits.T1IF = 0;// clear Timer1 interrupt flag
    IEC0bits.T1IE = 0;// disable Timer1 interrupt

    timer_start();
}

void serial_stop()
{
    U1MODEbits.UARTEN = 0; // controllare..
}


/*
 * tutta la prima parte (e l'ulitma) commentata deve essere riadattata una volta che
 * si conoscono i pin da usare ;-)
 */

void serial2_start(int use_interrupt, int baud_rate)
{
    //INIZIALIZZAZIONE SERIALE

    // MICROBRAIN PCB
    // TX = RB12/RP12
    // RX = RB7/RP7
    //set no open drain configuration
    ODCBbits.ODCB12 = 0; //Tx

    //set tx_pin as output
    TRISBbits.TRISB12 = PORT_OUT;
    //set rx_pin as input
    TRISBbits.TRISB7 = PORT_IN;

    //Assign rx,tx to the corresponding pins
    RPINR19bits.U2RXR = 0b0111; //U2->RX Rp
    RPOR6bits.RP12R = 0b00101; //U2TX -> Rp7

    //SET SERIAL
    U2MODEbits.STSEL = 0; // 1-stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U2MODEbits.ABAUD = 0; // Auto-Baud Disabled
    U2MODEbits.BRGH = 0; // Low Speed mode
    U2BRG = baud_rate; // BAUD Rate Setting for 115200

    U2MODEbits.UEN = 0b00;
    U2STAbits.URXISEL = 0b00;

    U2STAbits.UTXISEL0 = 0;
    U2STAbits.UTXISEL1 = 0;

    //#ifndef DEBUG_232
    IPC7bits.U2RXIP = 0b111; // set hightest level priority
    IFS1bits.U2RXIF = 0; // clear interrupt flag
    IEC1bits.U2RXIE = use_interrupt; // set RX interrupt

    IEC1bits.U2TXIE = 0; // no TX interrupt

    //#endif

    //IPC2bits.U1RXIP = 2;
    U2MODEbits.UARTEN = 1; // Enable UART

    U2STAbits.UTXEN = 1; // Enable UART Tx

    //LATBbits.LATB7 = 0; //Scrittura su 485 disabilitata
    //LATBbits.LATB11 = 1; //Tx line set to 1
}

void serial2_stop()
{
    U2MODEbits.UARTEN = 0;
}


void __attribute__((__interrupt__, __auto_psv__, __shadow__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    ++timer_tick;
}


void serial_output(char *uBuffer, int uBufferLength)
{
    int i;
    IO485_ENABLE_1();
    for (i = 0; i < uBufferLength; i++) {
        while (U1STAbits.TRMT == 0);//se trmt == 1 il buffer è vuoto
        U1TXREG = uBuffer[i]; //invio il char i-esimo
    }
    IO485_DISABLE_1();
}


int serial_input(unsigned char* buffer)
{

    int nreads = 0;
    unsigned char tmp;

    timer_start();
    if(serial_getc(&tmp) == -1) {
        timer_stop();
        return -1;
    }
    while(tmp!='\r') {
        buffer[nreads++] = tmp;
        if (serial_getc(&tmp) == -1) {
            timer_stop();
            return -1;
        }
    }
    buffer[nreads++] = 0; // discard CR
    timer_stop();
    return nreads;
}


int serial_getc(unsigned char * c)
{
    timer_reset();
    while (U1STAbits.URXDA == 0) {
        if (U1STAbits.OERR == 1) {
            U1STAbits.OERR = 0; // clear overrun if it occurs
        }
        if (is_timeout()) {
            return -1;
        }
    }
    *c = U1RXREG;

    printf("%02X:", *c);

    return 1;
}


int serial_getbuffer(unsigned char * buf, int size)
{

    int r;
    int err;
    timer_start();
    for (r = 0; r < size; r++) {
        if((err=serial_getc(&buf[r]))!=1) {
            timer_stop();
            return err;
        }
    }
    timer_stop();
    return r;

}

void __attribute__((__interrupt__, __auto_psv__, __shadow__)) _U1RXInterrupt(void)
{
    unsigned char data;

    IFS0bits.U1RXIF = 0 ;	  // clear interrupt flag

    data = U1RXREG ;

    if (REPLY.reply_ready == 1)
        return;

    if (REPLY.reply_size == 0) {
        // we are waiting for the first character
        if  ( (data == '$') || (data == '>') || (data == '#') ) {
            REPLY.reply_string[0] = data;
            REPLY.reply_size = 1;
            return;
        }
    }
    else {
        // we are waiting for other characters
        if (data == '\r')  {  // CR, end of string
            REPLY.reply_string[REPLY.reply_size] = 0;
            REPLY.reply_ready = 1;
        }
        else {
            if (REPLY.reply_size < MAX_REPLY_SIZE) {
                REPLY.reply_string[REPLY.reply_size++] = data;
            }
            else {
                // REPLY too long, discard it
                REPLY.reply_size = 0;
                REPLY.reply_ready = 0;
            }
        }
    }
}


