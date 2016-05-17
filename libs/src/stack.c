#include <stddef.h>
#include "assert.h"
#include "common.h"
//#include "mem.h"
#include "stack.h"

#define T Stack_T

struct T {
  int count;
  struct elem {
    void *x;
    struct elem *next;
  } *head;
};

T Stack_new(void)
{
  T stk;

  //NEW(stk);
  stk = MALLOC(1, struct T);
  stk->count = 0;
  stk->head = NULL;

  return stk;
}

int Stack_empty(T stk)
{
  assert(stk);
  return stk->count == 0;
}

void Stack_push(T stk, void *x)
{
  struct elem *t;

  assert(stk);
  t = MALLOC(1, struct elem);
  //NEW(t);
  
  t->x = x;
  t->next = stk->head;
  stk->head = t;
  stk->count++;
}

void *Stack_pop(T stk)
{
  assert(stk);
  assert(stk->count > 0);

  struct elem *t = stk->head;
  stk->head = t->next;
  stk->count--;
  void *x = t->x;
  free(t);
  // FREE(t);
  
  return x;
}

void Stack_free(T *stk)
{
  assert(stk && *stk);

  for (struct elem *t = (*stk)->head, *u; t; t = u) {
    u = t->next;
    free(t);
  }
  
  free(*stk);
}
