#include "queue.h"

void init_queue(t_queue *q){
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
}

void init_node(t_node *n, int t, int area, int lenght, char from, char to){
	n->next = NULL;
	n->t = t;
	n->area = area;
	n->lenght = lenght;
	n->from = from;
	n->to = to;
}

void push(t_queue *q, t_node *n){
	if (q->size == 0){
		q->head = n;
		q->tail = n;
		q->size = 1;
	}
	else {
		q->tail->next = n;
		q->tail = n;
		q->size = q->size + 1;
	}
}

t_node * pop(t_queue *q){
	if(q->size == 0)return NULL;//nessun elemento
	else if (q->head == q->tail && q->head != NULL){//ultimo elemento
		t_node * tmp = q->head;
		q->head = NULL;
		q->tail = NULL;
		q->size = 0;
		return tmp;
	}
	else{//elemento generico
		t_node * tmp = q->head;
		q->head = q->head->next;
		tmp->next = NULL;
		q->size = q->size - 1;
		return tmp;
	}
}

t_node * first(t_queue *q){
	return q->head;
}
