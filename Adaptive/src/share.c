#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "stack.h"

extern Stack_T stk;

Suf_Node_T cut_head(Suf_Node_T suf_node, Pat_Len_T lss)
{
  Pat_Len_T suf_len;
  
  if ((suf_len = strlen(suf_node->str)) == lss) { /* 该后缀无法再继续分割 */
    free(suf_node);
    return NULL;
  }

  memmove(suf_node->str, suf_node->str + lss, suf_len - lss + 1);
  
  return suf_node;
}

inline bool same_str(Char_T const *s1, Char_T const *s2, Pat_Len_T len)
{
  while (len && *s1 == *s2)
    len--, s1++, s2++;
  
  return !len;
}

/* 1 <= block_size <= 4 */
uint32_t block_123(UC_T const *p, int8_t block_size)
{
  uint32_t v = 0;
  
  while (block_size--) {
    v <<= BITS_PER_BYTE;
    v += *p++;
  }

  return v;
}

void push_children(Tree_Node_T child, Pat_Num_T num)
{
  while (num--) {
    if (child->link)
      Stack_push(stk, child);
    child++;
  }
}

#if DEBUG
void print_str(Char_T const *s, Pat_Len_T len, Char_T terminator)
{
  while (len--)
    putchar(*s++);

  putchar(terminator);
}

void print_suffix(Suf_Node_T cur_suf)
{
  while (cur_suf) {
    printf("    %s\n", cur_suf->str);
    cur_suf = cur_suf->next;
  }
}

#endif 
