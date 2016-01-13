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

static Single_Str_t *make_single_str(Pat_Len_t str_len)
{
     Single_Str_t *new_single_str = VMALLOC(Single_Str_t, Char_t, str_len);

     new_single_str->str_len = str_len;
     new_single_str->pat_end_flag = FALSE;
     new_single_str->expand_node.type = END;
     new_single_str->expand_node.next_level = NULL;

     return new_single_str;
}

static void build_single_str(Expand_Node_t *expand_node, Pat_Len_t str_len)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Single_Str_t *single_str = make_single_str(str_len);

  /* 拷贝第一个后缀 */
  cur_suf = expand_node->next_level;
  memcpy(single_str->str, cur_suf->str, str_len);
  next_p = (Suffix_Node_t **) &single_str->expand_node.next_level;

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    if (cur_suf = cut_head(cur_suf, str_len)) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      single_str->pat_end_flag = TRUE;
  }

  *next_p = NULL;

  expand_node->next_level = single_str;
  expand_node->type = SINGLE_STR;

  push_queue(&single_str->expand_node, 1);
}

static Str_Array_t *make_str_array(Pat_Num_t str_num, Pat_Len_t str_len)
{
  Str_Array_t *new_array = VMALLOC(Str_Array_t, Char_t, str_len * str_num);

  new_array->str_num = str_num;
  new_array->str_len = str_len;
  new_array->expand_nodes = CALLOC(str_num, Expand_Node_t);
  memset(new_array->str_buf, 0, str_num * str_len);
  if (str_num > POINTER_SIZE * BITS_PER_BYTE)
    new_array->pat_end_flag.p = CALLOC(str_num / BITS_PER_BYTE + (str_num % BITS_PER_BYTE) ? 1 : 0, Flag_t);
  else
    memset(new_array->pat_end_flag.flag, 0, POINTER_SIZE);

  return new_array;
}

static void build_str_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
  Suffix_Node_t *cur_suf, *next_suf,  **next_p;
  Str_Array_t *str_array = make_str_array(str_num, str_len); /* 构建str_array */
  Char_t *str_buf = str_array->str_buf;
  Expand_Node_t *cur_expand_node = str_array->expand_nodes;
  Flag_t *pat_end_flag;

  pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;

  cur_suf = expand_node->next_level;
  memcpy(str_buf, cur_suf->str, str_len);
  next_p = (Suffix_Node_t **) &cur_expand_node->next_level;

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    if (!same_str(str_buf, cur_suf->str, str_len)) {
      memcpy(str_buf += str_len, cur_suf->str, str_len); /*原链表终止, 指向新链表头 */
      *next_p = NULL; next_p = (Suffix_Node_t **) &(++cur_expand_node)->next_level;
    }

    if (cur_suf = cut_head(cur_suf, str_len)) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(pat_end_flag, cur_expand_node - str_array->expand_nodes);
  }

  *next_p = NULL;

  expand_node->next_level = str_array;
  expand_node->type = ARRAY;

  push_queue(str_array->expand_nodes, str_array->str_num);
}

void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
#if DEBUG
     array_size[str_num].num_1 = str_num;
     array_size[str_num].num_2++;
#endif

     if (str_num == 1) {
	  build_single_str(expand_node, str_len);
#if DEBUG
	  type_num[SINGLE_STR].num++;
#endif
     } else {
	  build_str_array(expand_node, str_num, str_len);
#if DEBUG
	  type_num[ARRAY].num++;
#endif
     }
}

inline Expand_Node_t *match_single_str(Single_Str_t *single_str, Char_t const **pos_p, Bool_t *is_pat_end)
{
#if DEBUG
 fun_calls[MATCH_SINGLE_STR].num++;
#endif

  if (!same_str(single_str->str, *pos_p, single_str->str_len))
    return NULL;

  *is_pat_end = single_str->pat_end_flag;
  *pos_p += single_str->str_len;

  return &single_str->expand_node;
}

/*有序查找*/
inline static Expand_Node_t *array_ordered_match(Str_Array_t *str_array, Char_t const **pos_p, Bool_t *is_pat_end)
{
#if DEBUG
  fun_calls[MATCH_ORDERED_ARRAY].num++;
#endif

  Pat_Len_t str_len = str_array->str_len;
  Pat_Num_t str_num = str_array->str_num;
  Char_t *str_buf = str_array->str_buf;
  Char_t const *p = *pos_p;
  int result;
  Flag_t *pat_end_flag;
  Pat_Num_t i;

  pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;

  while (str_num && (result = str_n_cmp(str_buf, p, str_len)) < 0)
    str_num--, str_buf += str_len;

  if (str_num == 0 || result > 0) 	/* 没找到 */
    return NULL;

  i = str_array->str_num - str_num; /* 第i个字符串匹配成功 */
  *is_pat_end = test_bit(pat_end_flag, i);
  *pos_p += str_len;

  return str_array->expand_nodes + i;
}

/* 二分查找 */
inline static Expand_Node_t *array_binary_match(Str_Array_t *str_array, Char_t const **pos_p, Bool_t *is_pat_end)
{
#if DEBUG
  fun_calls[MATCH_BINARY_ARRAY].num++;
#endif

  int low = 0, high = str_array->str_num - 1, mid;
  Char_t *str_buf = str_array->str_buf;
  Pat_Len_t str_len = str_array->str_len;
  Char_t const *p = *pos_p;
  int result;
  Flag_t *pat_end_flag;

  pat_end_flag = str_array->str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag.p :
    str_array->pat_end_flag.flag;

  while (low <= high) {
    mid = (low + high) >> 1;
    if ((result = str_n_cmp(p, str_buf + mid * str_len, str_len)) == 0) {
      *is_pat_end = test_bit(pat_end_flag, mid);
      *pos_p += str_len;
      return str_array->expand_nodes + mid;
    } else if (result < 0)
      high = mid - 1;
    else
      low = mid + 1;
  }

  return NULL;
}

Expand_Node_t *match_array(Str_Array_t *str_array, Char_t const **pos_p, Bool_t *is_pat_end)
{
  return (str_array->str_num <= SMALL_ARRAY_SIZE) ?
    array_ordered_match(str_array, pos_p, is_pat_end) :
    array_binary_match(str_array, pos_p, is_pat_end);
}

#if DEBUG
/* void print_array(Str_Array_t *str_array) */
/* { */
/*   Str_Elmt_t *str_elmt; */
/*   Pat_Len_t str_len = str_array->str_len; */
/*   Pat_Num_t n; */

/* #define PRINT_ARRAY(pointer)                                                       \ */
/*   for (str_elmt = str_array->array, n = str_array->str_num; n; str_elmt++, n--) {  \ */
/*     print_str(str_elmt->str.pointer, str_len,  ':');                               \ */
/*     if (str_elmt->pat_end_flag)                                                    \ */
/*       putchar('*');                                                                \ */
/*     putchar('\n');                                                                 \ */
/*     print_suffix(str_elmt->expand_node.next_level);                                \ */
/*   } */

/*   if (str_len > POINTER_SIZE) */
/*     PRINT_ARRAY(p) */
/*   else */
/*     PRINT_ARRAY(buf) */
/* } */
#endif
