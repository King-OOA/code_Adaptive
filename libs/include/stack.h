#ifndef STACK_H
#define STACK_H
#include <stdio.h>

#define STACK_TYPE ARRAY


struct stack;
typedef struct stack Stack_t;
typedef  char *  STACK_ELEMENT_T;


Stack_t *create_stack(size_t size);
void destroy_stack(Stack_t *stack);
int stack_is_full(Stack_t *stack);
int stack_is_empty(Stack_t *stack);
void push(STACK_ELEMENT_T element, Stack_t *stack);
STACK_ELEMENT_T top(Stack_t *stack);
void pop(Stack_t *stack);
STACK_ELEMENT_T top_and_pop(Stack_t *stack);

#endif
