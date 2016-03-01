#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "queue.h"

extern Queue_t *queue;

Suffix_Node_t *cut_head(Suffix_Node_t *suf_node, Pat_Len_t lss)
{
  Pat_Len_t suf_len;
  
  if ((suf_len = strlen(suf_node->str)) == lss) { /* 该后缀无法再继续分割 */
    free(suf_node);
    return NULL;
  } 

  memmove(suf_node->str, suf_node->str + lss, suf_len - lss + 1);
  
  return suf_node;
}

inline Bool_t same_str(Char_t const *s1, Char_t const *s2, Pat_Len_t len)
{
  while (len && *s1 == *s2) 
    len--, s1++, s2++;
  
  return !len;
}

void push_queue(Expand_Node_t const *expand_node, Pat_Num_t num)
{
  while (num--) {
    if (expand_node->next_level)
      in_queue(queue, expand_node);
    expand_node++;
  }
}

#if DEBUG
void print_str(Char_t const *s, Pat_Len_t len, Char_t terminator)
{
  while (len--)
    putchar(*s++);

  putchar(terminator);
}

void print_suffix(Suffix_Node_t *cur_suf)
{
  while (cur_suf) {
    printf("    %s\n", cur_suf->str);
    cur_suf = cur_suf->next;
  }
}

#endif 
