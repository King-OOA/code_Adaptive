#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"

void get_num_and_lsp(Expand_Node_t const *expand_node, Pat_Num_t *total_suf_p, Pat_Num_t *dif_prf_p, Pat_Len_t *lsp_p)
{
  Suffix_Node_t *cur_suf;
  Pat_Len_t suf_len, lsp = 255;
  Pat_Num_t num = 0, i;
  Char_t *buf, *s;
  
  /* 确定total_suf_num和lsp */
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = cur_suf->next) {
    num++;
    if ((suf_len = strlen(cur_suf->str)) < lsp)
      lsp = suf_len;
  }
  
  *lsp_p = lsp;
  *total_suf_p = num;
  
  /* 确定dif_prf_num */
  buf = MALLOC(lsp * num, Char_t);
  num = 0;
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = cur_suf->next) { /* 去重统计 */
    for (i = 0, s = buf; i < num && !same_str(s, cur_suf->str, lsp); i++, s += lsp)
      ;
    
    if (i == num) {
      memcpy(s, cur_suf->str, lsp);
      num++;
    }
  }
  
  *dif_prf_p = num;
  
  free(buf);
}

Suffix_Node_t *cut_head(Suffix_Node_t *suf_node, Pat_Len_t lsp)
{
  Pat_Len_t suf_len;
  
  if ((suf_len = strlen(suf_node->str)) == lsp) { /* 该后缀无法再继续分割 */
    free(suf_node);
    return NULL;
  } 
  
  memmove(suf_node->str, suf_node->str + lsp, suf_len - lsp + 1);
  
  return suf_node;
}

void insert_to_expand(Expand_Node_t *expand_node, Suffix_Node_t *suf_node)
{
  Suffix_Node_t **next_p = (Suffix_Node_t **) &expand_node->next_level;
  Suffix_Node_t *cur_suf;
  int result;
  
  while ((cur_suf = *next_p) && (result = strcmp(cur_suf->str, suf_node->str)) < 0)
    next_p = &cur_suf->next;
  
  if (cur_suf == NULL || result > 0) {
    suf_node->next = cur_suf;
    *next_p = suf_node;
  } else  /* 已经存在 */
    free(suf_node);
}

inline int same_str(Char_t const *s1, Char_t const *s2, Pat_Len_t len)
{
  while (len && *s1 == *s2) 
    len--, s1++, s2++;
  
  return len == 0 ? TRUE : FALSE;
}

void print_str(char const *s, Pat_Len_t len, char terminator)
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
