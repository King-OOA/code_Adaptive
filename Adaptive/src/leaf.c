#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "leaf.h"

void build_leaf(Expand_Node_t *expand_node)
{
     unsigned suf_len_sum = 0;
     Pat_Num_t suf_num = 0;
     Leaf_t *leaf;
     Suffix_Node_t *cur_suf, *next_suf;
     Pat_Len_t suf_len;
     Char_t *leaf_str;
     
     /* 统计串个数和串长和 */
     for (cur_suf = expand_node->next_level; cur_suf; cur_suf = cur_suf->next) {
	  suf_num++;
	  suf_len_sum += strlen(cur_suf->str);
     }
     
     leaf = VMALLOC(Leaf_t, Char_t, suf_len_sum + suf_num);
     leaf->suf_num = suf_num;
     
     leaf_str = leaf->str;
     for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
	  next_suf = cur_suf->next;
	  suf_len = strlen(cur_suf->str);
	  *leaf_str++ = suf_len;
	  memcpy(leaf_str, cur_suf->str, suf_len);
	  leaf_str += suf_len;
	  free(cur_suf);
     }
     
     expand_node->next_level = leaf;
     expand_node->type = LEAF;
}

void print_leaf(Leaf_t *leaf)
{
     unsigned char suf_num = leaf->suf_num;
     unsigned char suf_len;
     unsigned char *p = leaf->str;
     
     printf("#leaf#\n");
     while (suf_num--) {
	  suf_len = *p++;
	  putchar('\t');
	  print_str(p, suf_len, '\n');
	  p += suf_len;
     }
}
