#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "queue.h"
#include "array.h"


extern Queue_t *queue;
extern Pat_Num_t type_num[];

static Str_Array_t *make_array(Pat_Num_t str_num, Pat_Len_t str_len)
{
     Str_Array_t *new_array = VMALLOC(Str_Array_t, Str_Elmt_t, str_num);
     
     new_array->str_num = str_num;
     new_array->str_len = str_len;
     memset(new_array->array, 0, str_num * sizeof(Str_Elmt_t));
     
     return new_array;
}

/* 无序插入 */
void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
     Suffix_Node_t *cur_suf, *next_suf;
     Str_Array_t *str_array = make_array(str_num, str_len); /* 构建str_array */
     int n;
     Str_Elmt_t *str_elmt;
     
     if (str_len > POINTER_SIZE)  /* 节点外 */
	  for (str_num = 0, cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /* str_num表示当前在数组中已存放的串数 */
	       next_suf = cur_suf->next;
	       
	       for (n = str_num, str_elmt = str_array->array; n && !same_str(str_elmt->str.p, cur_suf->str, str_len); str_elmt++, n--)
		    ;

	       if (n == 0) {/* 没找到 */
		    str_elmt->str.p = MALLOC(str_len, Char_t);
		    memcpy(str_elmt->str.p, cur_suf->str, str_len);
		    str_num++;
	       }
	       
	       if (cur_suf = cut_head(cur_suf, str_len))
		    insert_to_expand(&str_elmt->expand_node, cur_suf);
	       else
		    str_elmt->pat_end_flag = 1;
	  }
     else  /* 节点内 */
	  for (str_num = 0, cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /* str_num表示当前在数组中已有的串数 */
	       next_suf = cur_suf->next;
	       
	       for (n = str_num, str_elmt = str_array->array; n && !same_str(str_elmt->str.buf, cur_suf->str, str_len); str_elmt++, n--)
		    ;

	       if (n == 0) {/* 没找到 */
		    memcpy(str_elmt->str.buf, cur_suf->str, str_len);
		    str_num++;
	       }
	       
	       if (cur_suf = cut_head(cur_suf, str_len))
		    insert_to_expand(&str_elmt->expand_node, cur_suf);
	       else
		    str_elmt->pat_end_flag = 1;
	  }

     expand_node->next_level = str_array;
     expand_node->type = ARRAY;
     type_num[ARRAY]++;
     
     /* 加入到队列 */
     for (str_elmt = str_array->array; str_num; str_elmt++, str_num--)
	  if (str_elmt->expand_node.next_level)
	       in_queue(queue, &str_elmt->expand_node);
}

Expand_Node_t *match_array(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end)
{
     Pat_Len_t str_len = str_array->str_len;
     Pat_Num_t str_num;
     Char_t const *s = *text;
     Str_Elmt_t *str_elmt;

     if (str_len > POINTER_SIZE)  /* 节点外 */
	  for (str_num = str_array->str_num, str_elmt = str_array->array;
	       str_num && !same_str(s, str_elmt->str.p, str_len);
	       str_num--, str_elmt++)
	       ;
     else 			/* 节点内 */
	  for (str_num = str_array->str_num, str_elmt = str_array->array;
	       str_num && !same_str(s, str_elmt->str.buf, str_len);
	       str_num--, str_elmt++)
	       ;

     if (str_num == 0) 	/* 没找到 */
	  return NULL;

     *is_pat_end = str_elmt->pat_end_flag;
     *text += str_len;
     
     return &str_elmt->expand_node;
}


/* void print_leaf(Leaf_t *leaf) */
/* { */
/*      unsigned char suf_num = leaf->suf_num; */
/*      unsigned char suf_len; */

/*      for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/* 	  next_suf = cur_suf->next; */
/* 	  suf_len = strlen(cur_suf->str); */
/* 	  *leaf_str++ = suf_len; */
/* 	  memcpy(leaf_str, cur_suf->str, suf_len); */
/* 	  leaf_str += suf_len; */
/* 	  free(cur_suf); */
/*      } */
/*      unsigned char *p = leaf->str; */
     
/*      printf("#leaf#\n"); */
/*      while (suf_num--) { */
/* 	  suf_len = *p++; */
/* 	  putchar('\t'); */
/* 	  print_str(p, suf_len, '\n'); */
/* 	  p += suf_len; */
/*      } */
/* } */
