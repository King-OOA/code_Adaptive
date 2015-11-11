#include <stdlib.h>
#include <stdio.h>
#include "sorter.h"

#define ALPHABET_SIZE 256
#define STACK_SIZE 5000000

static struct stack_item {
    List_Node_t *head, *tail;
    int dep;
} stack[STACK_SIZE], *sp = stack;
 
#define PUSH(list_head, list_tail, depth) (sp->head = list_head, sp->tail = list_tail, (sp++)->dep = depth)
#define POP(list_head, list_tail, depth)  (list_head = (--sp)->head, list_tail = sp->tail, depth = sp->dep)
#define STACKEMPTY() (sp <= stack)
#define SINGLETON(list_head) (list_head->next == NULL)
#define ENDED(list_head, depth) (depth > 0 && list_head->str[depth-1] == '\0')

List_Node_t * list_radix_sort(List_Node_t *list_head)
{
    static List_Node_t *pile[ALPHABET_SIZE], *tail[ALPHABET_SIZE];
    List_Node_t *list_tail, *sequel = NULL;
    int depth, nc = 0;
    UC_t ch, ch_min;

    if (list_head == NULL)
      return list_head;
    
    if (SINGLETON(list_head))
      return list_head;

    PUSH(list_head, NULL, 0);
    while (!STACKEMPTY()) {
        POP(list_head, list_tail, depth);
        if (SINGLETON(list_head) || ENDED(list_head, depth)) {
            list_tail->next = sequel;
            sequel = list_head;
            continue;
        }
        
        for (ch_min = 255; list_head; list_head = list_head->next) {
            ch = list_head->str[depth];
            if (pile[ch] == 0) {
                tail[ch] = pile[ch] = list_head;
                if (ch == 0) continue;
                if (ch < ch_min) ch_min = ch;
                nc++;
            } else
                tail[ch] = tail[ch]->next = list_head;
        }
        
        if (pile[0] != NULL) {
            PUSH(pile[0], tail[0], depth+1);
            tail[0]->next = pile[0] = NULL;
        }

        for (ch = ch_min; nc > 0; ch++)
            if (pile[ch]) {
                PUSH(pile[ch], tail[ch], depth+1);
                tail[ch]->next = pile[ch] = NULL;
                nc--;
            }
    }
    
    return sequel;
}
