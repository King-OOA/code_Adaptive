#pragma once

#include <stdbool.h>

#define Queue_T T


typedef struct T *T;

extern T Queue_new(void);
extern bool Queue_empty(T);
extern void Queue_push(T q, void *x);
extern void *Queue_pop(T q);
extern void Queue_free(T *q);

#undef T
