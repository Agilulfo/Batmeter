/*
 * defines.h
 */

#ifndef __DEFINES_H
#define __DEFINES_H

#define PORT_OUT  0
#define PORT_IN   1


#define USE_ECAN

#define RP(N)       N

#define NO_OPEN_DRAIN  0
#define OPEN_DRAIN     1


#define ON		1
#define OFF		0

#define OK		1
#define WARNING	0

#define TRUE 	1
#define FALSE	0

#define INSIDE 	1
#define OUTSIDE	0

//define batmeter



#define N_SENS 12 //sensori totali (numero pari in quanto divisi in due file)

//~ da 0 a N_SENS/2 -1 si hanno i sensori del lato interno 
//~ da N_SENS/2 fino a N_SENS -1 si hanno i sensori esterni

#define QUEUETIMEOUT 200 // TODO: definire tale valore sperimentalmente
#define RESET_TIME 1000 // TODO: mettere un valore più alto

#define FREQ_CAMPIONAMENTO 272 // campionamento ogni 0.0072 Sec. 
//(5 campionamenti per un pipistrello lungo 5 cm che viaggia a 50 km/h)

//#define FREQ_CAMPIONAMENTO 3000 //test value


//#define EXTERNAL_CLOCK
#define INTERNAL_CLOCK

#if defined(EXTERNAL_CLOCK) && defined(INTERNAL_CLOCK)
#error Both EXTERNAL_CLOCK and INTERNAL_CLOCK defined
#endif

#if !defined(EXTERNAL_CLOCK) && !defined(INTERNAL_CLOCK)
#error None of EXTERNAL_CLOCK and INTERNAL_CLOCK defined
#endif


#ifdef EXTERNAL_CLOCK
#define FCY   (77414400 / 2)
#endif



#ifdef INTERNAL_CLOCK
#define FCY   (77385000 / 2)
#endif

typedef unsigned char uint8_t;

#endif
