/*
 * main.c
 */

#include "defines.h"

#include <xc.h>
#include <p33FJ128MC802.h>
#include <libpic30.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "clocking.h"
#include "led.h"
#include "serial_driver.h"
#include "gpio.h"
#include "queue.h"

//#define ININTERRUPTPRINTS 

_FPOR(PWMPIN_ON & FPWRT_PWR1 & ALTI2C_OFF);

_FICD(ICS_PGD1 & JTAGEN_OFF);

_FWDT(FWDTEN_OFF);

int batCounterExit;
int batCounterEntrance;
int lastPrintedExit;
int lastPrintedEntrance;
unsigned char raw_packet;

int t;
int start [N_SENS]; //default -1
int end [N_SENS];   //default -1
char old [N_SENS];  //default FALSE 
char new [N_SENS];  //default FALSE
char busy [N_SENS]; //default FALSE
unsigned char rawmode;

t_queue *insideQueue;
t_queue *outsideQueue;

void read_sensors(char * v){
	v[0] = ! GPIO_3; //in 1
	v[1] = ! GPIO_4; //in 2
	v[2] = ! GPIO_2; //in 3
	
	v[3] = ! GPIO_0; //in 4
	v[4] = ! GPIO_1; //in 5
	v[5] = ! GPIO_5; //in 6
	
	v[6] = ! GPIO_11; //out 1
	v[7] = ! GPIO_7; //out 2
	v[8] = ! GPIO_6; //out 3
	
	v[9] = ! GPIO_8; //out 4
	v[10] = ! GPIO_9; //out 5
	v[11] = ! GPIO_10; //out 6
}

void tryReset(){
	int i;
	int c = 0;
	for(i = 0 ; i < N_SENS;i++){
		c+= busy[i];
	}
	
	//se c == 0 vuol dire che non ci sono run in corso! quindi ...
	if(c == 0 && (outsideQueue->size == 0 && insideQueue->size == 0)){
		
#ifdef ININTERRUPTPRINTS
		printf("reset T\n\r");
#endif
		t=0;
	}
}



char adjRuns(int sa, int ea, int sb, int eb){//e->end s->start 
	/*due serie non sono adj solo quando una si chiude 
	 * prima che inizi l'altra*/
	if (ea == -1) ea = t;
	if (eb == -1) eb = t;
	if (ea < sb || sa > eb) return FALSE;
	else return TRUE;
	
}

void findSequences(char side){
	unsigned char index = (side == INSIDE)? 0 : N_SENS/2;
	char stop = (side == INSIDE)? N_SENS/2-1 : N_SENS-1;
	
	char from;
	char to;
	
	while(index <= stop){// per ogni iterazione trovo una sequenza
		from = -1;
		to = -1;
		//cerco l'inizio della seq
		while(index <= stop){
			if(busy[index]){
				from = index;
				if(index == stop) to = index;
				index++;
				break;
			}
			index++;
		}
		
		if(from ==  -1) break; // vuol dire che si è arrivati a stop 
		//makwsenza trovare l'inizio di una nuova seq
		
		//cerco la fine della seq
		//arrivo a questo punto solo se from è diverso da -1
		
		/*
		 * trovo la fine di una sequenza in uno dei seguenti casi:
		 *  1) trovo un index per cui busy e' FALSE: in questo caso
		 * to è index -1
		 *  2) trovo un index per cui busy e' TRUE ma la cui run non è 
		 * compatibile con quella di index -1
		 *  3) trovo un index per cui busy e' TRUE, la run è compatibile
		 * ma si tratta dell'ultimo index considerabile*/
		while(index <= stop){
			if(busy[index]){
				if(!adjRuns(start[index-1], end[index-1], start[index], end[index])){//caso 2
					to = index-1;
					index++;
					break;
				}
				else if(index == stop){//caso 3
					to = index;
					index++;
					break;
				}		
			}
			else{ //caso 1
				to = index-1;
				index++;
				break;
			}
			index++;
		}
		
		//a questo punto from e to devono necessariamente essere diversi
		//da -1
		
		
		//controllo se tutte le run della sequenza sono chiuse
		int i;
		int count = 0;
		for(i = from; i<=to; i++)if(end[i] != -1)count++;
		
		if(count== to - from +1){//else ignoro la sequenza rimandandola allo step successivo
#ifdef ININTERRUPTPRINTS
			printf("someting detected in side: %d \n\r", side);
#endif
			int area = 0;
			int lenght = 0;
			
			for(i = from; i<=to; i++){
				int lenRun = end [i] - start[i] + 1; 
				area += lenRun;
				if (lenRun > lenght) lenght= lenRun;
			}
			
			t_node *newNode = malloc(sizeof(t_node));
			init_node(newNode, t, area, lenght, from, to);
			
			if(side == INSIDE) push(insideQueue,newNode); 
			else push(outsideQueue,newNode);
			
			//reset seq. variables
			for(i = from; i<=to; i++){
				start[i] = -1;
				end[i] = -1;
				busy[i] = FALSE;
			} 
		}
	}
}

void manageQueues(){
	while(insideQueue->size > 0 && outsideQueue->size > 0){
		int ti, to;
		ti = insideQueue->head->t;
		to = outsideQueue->head->t;
		
		if(ti > to){
			batCounterEntrance ++;
#ifdef ININTERRUPTPRINTS
			printf("%d ^^*^^ <---- entra\n\r", batCounterEntrance);
#endif
		}
		else{
			batCounterExit ++;
#ifdef ININTERRUPTPRINTS
			printf("%d ^^*^^ ----> esce\n\r", batCounterExit);
#endif
			
		}
		t_node *tmp = pop(insideQueue);
		free(tmp);
		tmp = pop(outsideQueue);
		free(tmp);
	}
	
	
	//GESTIONE TIMEOUT
	if(insideQueue->size == 0 && outsideQueue->size > 0){//check timeout in outside
		while(outsideQueue->size > 0 && (t - outsideQueue->head->t) > QUEUETIMEOUT){
			t_node *tmp = pop(outsideQueue);
			if(tmp != NULL){
				free(tmp);
#ifdef ININTERRUPTPRINTS
				printf("timeout coda OUTSIDE\n\r");
#endif
			}
		}
	}
	else if(insideQueue->size > 0 && outsideQueue->size == 0){//check timeout in inside
		while(insideQueue->size > 0 &&(t - insideQueue->head->t) > QUEUETIMEOUT){
			t_node *tmp = pop(insideQueue);
			if(tmp != NULL){
				free(tmp);
#ifdef ININTERRUPTPRINTS
				printf("timeout coda INSIDE\n\r");
#endif
			}
		}
	}
}

void init_timer(void) {

    T2CONbits.TON = 0; // disabilito il timer2

    T2CONbits.TSIDL = 0;

    T2CONbits.T32 = 0; //32 bit mode on
    T2CONbits.TCS = 0;
    T2CONbits.TGATE = 0;

    T2CONbits.TCKPS = 0b11; // 1:256 prescaler

    TMR2 = 0;
    //TMR2 si incrementa ogni 1/37780 sec.

    PR2 = FREQ_CAMPIONAMENTO; //TODO: settare correttamente

    IPC1bits.T2IP = 1; // priorita' a 1
    IEC0bits.T2IE = 1; // disabilito Timer2 interrupt
    IFS0bits.T2IF = 0; // reset del dell' interrupt flag

    T2CONbits.TON = 0; // disabilito il timer2

}

void start_timer(void) {
	T2CONbits.TON = 1; // abilito il timer2
}

void stop_timer(void){
	T2CONbits.TON = 0; // disabilito il timer2
}

void __attribute__((__interrupt__, __auto_psv__, __shadow__)) _T2Interrupt(void) {
    //IFS0bits.T2IF = 0; // reset del dell' interrupt flag
	//led_toggle();
	read_sensors(new);
	
	
	//aggiornamento di start ed end in base ai cambiamenti
	
	//~ quattro possibili stati:
	//~ 1) nuovo on vecchio off		aggiorno start
	//~ 2) nuovo on vecchio on		
	//~ 3) nuovo off vecchio on		aggionro end
	//~ 4) nuovo off vecchio off
	
	char chg = FALSE;
	
	
	int i;
	for (i = 0; i < N_SENS;i++){
		if(new[i] == 1){
			if(old[i] == 0 && start[i] == -1){//caso 1
			/*
			* questo controllo ci assicura che sullo stesso sensore non
			* vengono catturate ulteriori run finche la sequenza non 
			* e stata riconosciuta e quindi vengono resettati i valori 
			* di start ed end
			*/
			
				start[i] = t;
				busy[i] = TRUE; 
				//chg = TRUE;
			}
		}
		else{// new[i] == 0
			if(old[i] == 1 && end[i] == -1){//caso 3
				if(start[i] != -1){
					end[i] = t; 
					chg = TRUE;
				}
			}
		}
	}
	
	if(chg){
		findSequences(INSIDE);
		findSequences(OUTSIDE);
	}
	
	manageQueues();
	
	t++;
	if(t>RESET_TIME)tryReset();// se non vi sono rilevazioni in corso resettiamo t
	
	// riempio old con i valori di new
	for(i = 0; i<N_SENS ;i++) old[i] = new[i];
	
	if(raw_packet < 100){
		raw_packet++;
		unsigned int data = 0xA000;  // 1010 XXXX XXXX XXXX
		int i;
		for(i = 0 ; i<N_SENS ; i++){
			data |= new[i] << i;
		}
		// il bit meno significativo è il sensore "in1" il bit più significativo è il sensore "out6"
		printf("$%04X%04X%04X\n\r",data,batCounterExit,batCounterEntrance); // TODO Sostituire con invio dati a basso livello
	} 
	
	
	IFS0bits.T2IF = 0; // reset del dell' interrupt flag
}

void initialize_peripherals(void) {
    init_clock();

    IEC0 = 0; // DISABLE ALL USER INTERRUPT
    IEC1 = 0;
    IEC2 = 0;
    IEC3 = 0;
    IEC4 = 0;

    RCONbits.SWDTEN = 0; // disable Watchdog Timer


    /* inizialize BUMPER */

    AD1PCFGL = 0xff; //porte abilitate in digitale

    serial2_start(0, B115200);
	
	// setto tutti i pin ANx come digitali
    AD1PCFGL = 0xFFFF; // al reset vale 0x0000 (usare pin in mod. analog.)
	
	// pin come input
    
    GPIO_0_DIR = 1;
    GPIO_1_DIR = 1;
    GPIO_2_DIR = 1;
    GPIO_3_DIR = 1;
    GPIO_4_DIR = 1;
    GPIO_5_DIR = 1;
    GPIO_6_DIR = 1;
    GPIO_7_DIR = 1;
    GPIO_8_DIR = 1;
    GPIO_9_DIR = 1;
    GPIO_10_DIR = 1;
    GPIO_11_DIR = 1;
    

	init_timer();

    set_led();
    
}

int kbhit(void) {
    return U2STAbits.URXDA == 1;
}

char read_char(void) {
    while (U2STAbits.URXDA == 0) {
        if (U2STAbits.OERR == 1) {
            U2STAbits.OERR = 0; // clear overrun if it occurs
        }
    }
    return U2RXREG;
}

void read_line(char * s) {
    char * st = s;
    for (;;) {
        char c;
        c = read_char();
        if (c < ' ') {
            switch (c) {
                case 0x0d:
                    putchar(13);
                    putchar(10);
                    *s = 0;
                    return;
                case 0x08:
                    if (s != st) {
                        putchar(8);
                        putchar(0x20);
                        putchar(8);
                        --s;
                    }
                    break;
            }
        } else {
            putchar(c);
            *s = c;
            ++s;
        }
    }
}


void print_sensors(){
	//while(1){	
	//	__delay_ms(200);
		char sens [N_SENS];
		read_sensors(sens);
		int i ;
		printf("=======================\n\rINSIDE ");
		for(i = 0; i < N_SENS/2;i++){
			if(sens[i]){
				printf("| %d |", i + 1 );	
			}
			
		}
		printf("\n\r");
		
		printf("OUTSIDE");
		for(i = N_SENS/2; i < N_SENS;i++){
			if(sens[i]){
				printf("| %d |", i - 5 );
			}
		}
		printf("\n\r=======================\n\r");
	//}
}

void verbose(){
#ifndef ININTERRUPTPRINTS
	while(!kbhit()){
		if(lastPrintedEntrance< batCounterEntrance){
	
			
			printf("%d ^^*^^ <---- entra\n\r", batCounterEntrance);
			lastPrintedEntrance = batCounterEntrance;
		}
		
		if(lastPrintedExit<batCounterExit){
			printf("%d ^^*^^ ----> esce\n\r", batCounterExit);
			lastPrintedExit = batCounterExit;
		}
	}
#endif
}


int main(void) {
    
    int i;
    char line[90];

    __C30_UART = 2; // stdout onto UART2
	
	t = 0;
	batCounterExit = 0;
	batCounterEntrance = 0;
	lastPrintedExit = 0;
	lastPrintedEntrance = 0;
	
	rawmode = FALSE;
	
	
	insideQueue = malloc(sizeof(t_queue));
	outsideQueue = malloc(sizeof(t_queue));
	init_queue(insideQueue);
	init_queue(outsideQueue);
    
	
	for(i = 0; i < N_SENS; i++){
		start[i] = -1;
		end[i] = -1;
		old[i] = FALSE;
		new[i] = FALSE;
		busy[i] = FALSE;
	}
	
    initialize_peripherals();
    
    
    
    for (i = 0; i < 10; i++) {
        led_on();
        __delay_ms(200);
        led_off();
    }

    printf("Starting\n\r");

    printf("^^*^^ bat bat\n\r");
    
    //avvio campionamento
	raw_packet = 100;
	start_timer();
        //-------------------
    	
    for (;;) {
        int argc;
        char *argv[20];
	
		
        printf("BatMeter>");
        read_line(line);
		
	
	
	
	argc = 0;
        argv[0] = strtok(line, " ");
        argc++;
        while ((argv[argc] = strtok(NULL, " ")) != NULL) argc++;

        if (!strcmp(argv[0], "menu") || !strcmp(argv[0], "help")) {
	puts("lon                          led  on");
	puts("lof                          led  off");
	puts("ver                          verbose mode");
	puts("raw                          raw mode");
	puts("start                        start timer");
	puts("stop                         stop timer");
	puts("get                          print sensors status");
	} else if (!strcmp(argv[0], "lon") && argc == 1) {
		led_on();
        } else if (!strcmp(argv[0], "lof") && argc == 1) {
		led_off();
        } else if (!strcmp(argv[0], "ver") && argc == 1) {
		verbose();
	} else if (!strcmp(argv[0], "start") && argc == 1) {
		start_timer();
        } else if (!strcmp(argv[0], "stop") && argc == 1) {
		stop_timer();
        } else if (!strcmp(argv[0], "raw") && argc == 1) {
		raw_packet = 0;
        } else if (!strcmp(argv[0], "get") && argc == 1) {
		print_sensors();
	} else if (!strcmp(argv[0], "test") && argc == 1) {
			
        } else if (strlen(argv[0]) == 0) {
            continue;
        } else {
            puts("ERROR");
        }
        
        //----------------------------------------------------
        

        
    }

    printf("\n\rRebooting....\n\r\n\r");
    return 0;
}
