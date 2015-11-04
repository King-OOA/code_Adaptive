#ifndef	QUEUE_H
#define QUEUE_H

#include "share.h"

#define EMPTY NULL		

typedef Expand_Node_t* Q_Value_t; /*队列中的元素为状态的指针*/

typedef struct q_node {
	Q_Value_t value;
	struct q_node *next;
} Q_Node_t;

typedef struct queue {
	Q_Node_t *head;
	Q_Node_t *tail;
	unsigned count;
} Queue_t;


void test_queue(Queue_t *);
Queue_t *make_queue(void);
void free_queue(Queue_t *);
void in_queue(Queue_t *, Q_Value_t);
Q_Value_t out_queue(Queue_t *);
Q_Value_t get_q_first(Queue_t *);
Q_Value_t get_q_last(Queue_t *);
void print_queue(Queue_t *);
void clean_queue(Queue_t *);
int queue_is_empty(Queue_t *);

#endif
