#include <stdlib.h>

struct node{
	struct node *next;
	int t; 			//istante rilevazione
	int area;		//area della sequenza
	int lenght;		//lunghezza media delle run
	char from;		//indirizzo prima run
	char to;		//indirizzo ultima run
};

typedef struct node t_node;

typedef struct queue{
	t_node *head;
	t_node *tail;
	int size;
}t_queue;

void init_queue(t_queue *q);
void init_node(t_node *n, int t, int area, int lenght, char from, char to);
void push(t_queue *q, t_node *n);
t_node * pop(t_queue *q);
t_node * first(t_queue *svq);
