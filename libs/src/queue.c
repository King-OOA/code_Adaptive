#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
#include "queue.h"
#include "common.h"

#define T Queue_T

struct T {
  int count;
  struct elem {
    void *x;
    struct elem *next;
  } *head, *tail;
};

T Queue_new(void)
{
  T q;

  q = MALLOC(1, struct T);
  
  q->count = 0;
  q->tail = q->head = NULL;
  
  return q;
}

bool Queue_empty(T q)
{
  assert(q);
  
  return q->count == 0;
}

void Queue_push(T q, void *x)
{
  struct elem *t;

  assert(q);

  t = MALLOC(1, struct elem);
  t->x = x;
  t->next = NULL;
  
  if (Queue_empty(q))
    q->head = q->tail = t;
  else {
    q->tail->next = t;
    q->tail = t;
  }
  
  q->count++;
}

void *Queue_pop(T q)
{
  assert(q);
  assert(q->count > 0);

  struct elem *t = q->head;
  q->head = t->next;
  void *x = t->x;
  free(t);
  q->count--;
  
  if (Queue_empty(q))
    q->tail = NULL;
  
  return x;
}

void Queue_free(T *q)
{
  assert(q && *q);
  
  for (struct elem *t = (*q)->head, *u; t; t = u) {
    u = t->next;
    free(t);
  }

  free(*q);
}
