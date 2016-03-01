#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "queue.h"
#include "array.h"
#include "statistics.h"
#include "binary.h"

extern Queue_t *queue;
extern Str_Num_t type_num[];
extern Str_Num_t fun_calls[];
extern Num_Num_t array_size[];

static Expand_Node_t *match_single_str(Single_Str_t *single_str, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
 fun_calls[MATCH_SINGLE_STR].num++;
#endif

  if (!same_str(single_str->str, *pos_p, single_str->str_len))
    return NULL;

  *is_pat_end_p = TRUE; /* 肯定是模式终止节点 */
  *pos_p += single_str->str_len;

  return single_str->expand_node;
}

/*有序查找*/
static Expand_Node_t *array_ordered_match(Str_Array_t *str_array, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_ORDERED_ARRAY].num++;
#endif

  Pat_Len_t str_len = str_array->str_len;
  Pat_Num_t str_num = str_array->str_num;
  Char_t *str_buf = str_array->str_buf;
  Char_t const *t_str = *pos_p;	/* 目标字符串 */
  int result;
  Flag_t *pat_end_flag;
  Pat_Num_t i;

  pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;

  while (str_num && (result = memcmp(str_buf, t_str, str_len)) < 0)
    str_num--, str_buf += str_len;

  if (str_num == 0 || result > 0) 	/* 没找到 */
    return NULL;

  i = str_array->str_num - str_num; /* 第i个字符串匹配成功 */
  *is_pat_end_p = test_bit(pat_end_flag, i);
  *pos_p += str_len;

  return str_array->expand_nodes + i;
}

/* 二分查找 */
static Expand_Node_t *array_binary_match(Str_Array_t *str_array, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_BINARY_ARRAY].num++;
#endif

  int low = 0, high = str_array->str_num - 1, mid;
  Char_t *str_buf = str_array->str_buf;
  Pat_Len_t str_len = str_array->str_len;
  Char_t const *t_str = *pos_p;
  int result;
  Flag_t *pat_end_flag;

  pat_end_flag = str_array->str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;

  while (low <= high) {
    mid = (low + high) >> 1;
    result = memcmp(t_str, str_buf + mid * str_len, str_len);
    
    if (result < 0)
      high = mid - 1;
    else if (result > 0)
      low = mid + 1;
    else {
      *is_pat_end_p = test_bit(pat_end_flag, mid);
      *pos_p += str_len;
      return str_array->expand_nodes + mid;
    } 
  }

  return NULL;
}

static Single_Str_t *make_single_str(Pat_Len_t str_len)
{
     Single_Str_t *new_single_str = VMALLOC(Single_Str_t, Char_t, str_len);

     new_single_str->str_len = str_len;
     new_single_str->expand_node->match_fun = NULL;
     new_single_str->expand_node->next_level = NULL;

     return new_single_str;
}

/* 所有后缀前str_len个字符相同, sing_str肯定是模式终止节点 */
static void build_single_str(Expand_Node_t *expand_node, Pat_Len_t str_len)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Single_Str_t *single_str = make_single_str(str_len);

  cur_suf = expand_node->next_level;
  memcpy(single_str->str, cur_suf->str, str_len);
  next_p = (Suffix_Node_t **) &single_str->expand_node->next_level;

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
#if DEBUG
    assert(same_str(cur_suf->str, single_str->str, str_len));
#endif 
    next_suf = cur_suf->next;
    if ((cur_suf = cut_head(cur_suf, str_len))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    }
  }
  
  *next_p = NULL;

  expand_node->next_level = single_str;
  expand_node->match_fun = (Match_Fun_t) match_single_str;

  push_queue(single_str->expand_node, 1);
}

static Str_Array_t *make_str_array(Pat_Num_t str_num, Pat_Len_t str_len)
{
  Str_Array_t *new_array = VMALLOC(Str_Array_t, Char_t, str_len * str_num);

  new_array->str_num = str_num;
  new_array->str_len = str_len;
  new_array->expand_nodes = CALLOC(str_num, Expand_Node_t); /* 外置扩展节点数组 */
  memset(new_array->str_buf, 0, str_num * str_len);
  if (str_num > POINTER_SIZE * BITS_PER_BYTE) /* 外置pat_end_flag */
    new_array->pat_end_flag.p = CALLOC(str_num / BITS_PER_BYTE + (str_num % BITS_PER_BYTE) ? 1 : 0, Flag_t);
  else /* 内置pat_end_flag */
    memset(new_array->pat_end_flag.flag, 0, POINTER_SIZE);

  return new_array;
}

static void build_str_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
  Suffix_Node_t *cur_suf, *next_suf,  **next_p;
  Str_Array_t *str_array = make_str_array(str_num, str_len); /* 构建str_array */
  Char_t *cur_str, pre_str[str_len], *str_buf = str_array->str_buf;
  Expand_Node_t *expand_nodes_array = str_array->expand_nodes;
  Flag_t *pat_end_flag;
  int i = -1;
  
  memset(pre_str, 0, str_len);

  pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;
  
  next_p = (Suffix_Node_t **) &expand_nodes_array[0].next_level;
  
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    if (!same_str((cur_str = cur_suf->str), pre_str, str_len)) {
      *next_p = NULL; memcpy(str_buf + (++i) * str_len, cur_str, str_len); /*原链表终止, 指向新链表头 */
      next_p = (Suffix_Node_t **) &expand_nodes_array[i].next_level;
      memcpy(pre_str, cur_str, str_len);
    }

    if ((cur_suf = cut_head(cur_suf, str_len))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(pat_end_flag, i);
  }

  *next_p = NULL;

#if DEBUG
  assert(str_num == i+1);
#endif 

  expand_node->next_level = str_array;
  expand_node->match_fun = (Match_Fun_t) (str_num > SMALL_ARRAY_SIZE ? array_binary_match : array_ordered_match);

  push_queue(expand_nodes_array, str_num);
}

void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
#if PROFILING
     array_size[str_num].num_1 = str_num;
     array_size[str_num].num_2++;
#endif

     if (str_num == 1) {
	  build_single_str(expand_node, str_len);
#if PROFILING
	  type_num[SINGLE_STR].num++;
#endif
     } else {
	  build_str_array(expand_node, str_num, str_len);
#if PROFILING
	  type_num[ARRAY].num++;
#endif
     }
}
