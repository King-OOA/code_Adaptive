#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "share.h"
#include "common.h"
#include "queue.h"
#include <stdint.h>
#include "array.h"
#include "statistics.h"
#include "binary.h"

extern Queue_T *queue;
extern Str_Num_T type_num[];
extern Str_Num_T fun_calls[];
extern Num_Num_T array_size[];
extern Num_Num_T array_len[];

typedef struct Single_Str {
  struct Tree_Node child;
  Pat_Len_T str_len;
  Char_T str[];
} *Single_Str_T;

typedef struct Str_Array {
  Pat_Num_T str_num;
  Pat_Len_T str_len;
  struct Tree_Node *children;	/* 外置扩展节点数组 */
  Flag_T *pat_end_flag;
  Char_T str_buf[];
} *Str_Array_T;

typedef struct Map_65536 {
  struct Tree_Node children[65536];
  Flag_T pat_end_flag[65536/8];
} *Map_65536_T;

static Tree_Node_T match_single_str(Single_Str_T single_str, Char_T const **pos_p, bool *pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_SINGLE_STR].num++;
#endif

  if (!same_str(single_str->str, *pos_p, single_str->str_len))
    return NULL;

  *pat_end_p = true; /* 肯定是模式终止节点 */
  *pos_p += single_str->str_len;

  return &single_str->child;
}

/*有序查找*/
static Tree_Node_T array_ordered_match(Str_Array_T str_array, Char_T const **pos_p, bool *pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_ORDERED_ARRAY].num++;
#endif

  Pat_Len_T str_len = str_array->str_len;
  Pat_Num_T str_num = str_array->str_num;
  Char_T *str_buf = str_array->str_buf;
  Char_T const *t_str = *pos_p;	/* 目标字符串 */
  int8_t result;

  Flag_T *pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag :
    (Flag_T *) &str_array->pat_end_flag;

  while (str_num && (result = memcmp(str_buf, t_str, str_len)) < 0)
    str_num--, str_buf += str_len;

  if (str_num == 0 || result > 0) 	/* 没找到 */
    return NULL;

  Pat_Num_T i = str_array->str_num - str_num; /* 第i个字符串匹配成功 */
  *pat_end_p = test_bit(pat_end_flag, i);
  *pos_p += str_len;

  return str_array->children + i;
}

/* 二分查找 */
static Tree_Node_T array_binary_match(Str_Array_T str_array, Char_T const **pos_p, bool *pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_BINARY_ARRAY].num++;
#endif

  int32_t low = 0, high = str_array->str_num - 1;
  Char_T *str_buf = str_array->str_buf;
  Pat_Len_T str_len = str_array->str_len;
  Char_T const *t_str = *pos_p;

  Flag_T *pat_end_flag = str_array->str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag :
    (Flag_T *) &str_array->pat_end_flag;

  while (low <= high) {
    int32_t mid = (low + high) >> 1;
    int8_t result = memcmp(t_str, str_buf + mid * str_len, str_len);
    
    if (result < 0)
      high = mid - 1;
    else if (result > 0)
      low = mid + 1;
    else {
      *pat_end_p = test_bit(pat_end_flag, mid);
      *pos_p += str_len;
      return str_array->children + mid;
    } 
  }

  return NULL;
}

static uint16_t block_2(UC_T const *p)
{
  uint16_t u = *p;

  return (u << BITS_PER_BYTE) + *(p + 1);
}

static Tree_Node_T match_map_65536(Map_65536_T map_65536, Char_T **pos_p, bool *pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_65536].num++;
#endif
   
  uint16_t t_block = block_2(*pos_p);
  Tree_Node_T child = map_65536->children + t_block;

  *pat_end_p = test_bit(map_65536->pat_end_flag, t_block);
  /* 匹配的三种情况: 1.只是终止节点,没有后续; 2.既是终止节点,又有后续; 3.不是终止节点,但有后续 */
  if (*pat_end_p || child->link)
    (*pos_p) += 2;

  return child->link == NULL ? NULL : child;
}

static Single_Str_T make_single_str(Pat_Len_T str_len)
{
     Single_Str_T new_single_str = VMALLOC(struct Single_Str, Char_T, str_len);

     new_single_str->str_len = str_len;
     new_single_str->child.match_fun = NULL;
     new_single_str->child.link = NULL;

     return new_single_str;
}

/* 所有后缀前str_len个字符相同, sing_str肯定是模式终止节点 */
static void build_single_str(Tree_Node_T t, Pat_Len_T str_len)
{
  Single_Str_T single_str = make_single_str(str_len);
  memcpy(single_str->str, ((Suf_Node_T) t->link)->str, str_len);

  struct Suf_Node **next_p = (struct Suf_Node **) &single_str->child.link;
  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG
    assert(same_str(cur_suf->str, single_str->str, str_len));
#endif 
    if ((cur_suf = cut_head(cur_suf, str_len))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    }
  }
  
  *next_p = NULL;

  t->link = single_str;
  t->match_fun = (Match_Fun_T) match_single_str;

  push_children(&single_str->child, 1);
}

static Str_Array_T make_str_array(Pat_Num_T str_num, Pat_Len_T str_len)
{
  Str_Array_T new_array = VMALLOC(struct Str_Array, Char_T, str_len * str_num);

  new_array->str_num = str_num;
  new_array->str_len = str_len;
  new_array->children = CALLOC(str_num, struct Tree_Node); /* 外置孩子节点数组 */
  memset(new_array->str_buf, 0, str_num * str_len);
  if (str_num > POINTER_SIZE * BITS_PER_BYTE) /* 外置pat_end_flag */
    new_array->pat_end_flag = CALLOC(str_num / BITS_PER_BYTE + (str_num % BITS_PER_BYTE) ? 1 : 0, Flag_T);
  else /* 内置pat_end_flag */
    memset(&new_array->pat_end_flag, 0, POINTER_SIZE);

  return new_array;
}

static void build_str_array(Tree_Node_T t, Pat_Num_T str_num, Pat_Len_T str_len)
{
  Str_Array_T str_array = make_str_array(str_num, str_len); /* 构建str_array */
  Char_T *cur_str, prev_str[str_len], *str_buf = str_array->str_buf; /* prev_str为变长数组 */
  int32_t i = -1;
  
  prev_str[0] = '\0';

  Flag_T *pat_end_flag = str_num > POINTER_SIZE * BITS_PER_BYTE ?
    str_array->pat_end_flag : (Flag_T *) &str_array->pat_end_flag;
  
  struct Suf_Node **next_p = (struct Suf_Node **) &str_array->children[0].link;
  
  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    if (!same_str((cur_str = cur_suf->str), prev_str, str_len)) {
      *next_p = NULL; memcpy(str_buf + (++i) * str_len, cur_str, str_len); /*原链表终止, 指向新链表头 */
      next_p = (struct Suf_Node **) &str_array->children[i].link;
      memcpy(prev_str, cur_str, str_len);
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

  t->link = str_array;
  t->match_fun = (Match_Fun_T) (str_num > SMALL_ARRAY_SIZE ? array_binary_match : array_ordered_match);

  push_children(str_array->children, str_num);
}

void build_map_65536(Tree_Node_T t)
{
  Map_65536_T map_65536 = CALLOC(1, struct Map_65536);

#if PROFILING
  type_num[MAP_65536].num++;
#endif

  struct Suf_Node **next_p = (struct Suf_Node **) &map_65536->children[0].link; /* 初始化 */
  uint16_t pre_block = 0, cur_block;

  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    if ((cur_block = block_2(cur_suf->str)) != pre_block) { /* 终止上一个链表,跳转到新链表头 */
#if DEBUG
      assert(cur_block > 0 && cur_block < 65536);
#endif 
     *next_p = NULL;
      next_p = (struct Suf_Node **) &map_65536->children[cur_block].link;
      pre_block = cur_block;
    }

    if ((cur_suf = cut_head(cur_suf, 2))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(map_65536->pat_end_flag, cur_block);
  }

  *next_p = NULL;

  t->link = map_65536;
  t->match_fun = (Match_Fun_T) match_map_65536;

  push_children(map_65536->children, 65536);
}

void build_array(Tree_Node_T t, Pat_Num_T str_num, Pat_Len_T str_len)
{
#if PROFILING
  /* 按数组元素个数分类 */
     array_size[str_num].num_1 = str_num;
     array_size[str_num].num_2++;
  /* 按数组元素长度分类 */
     array_len[str_len].num_1 = str_len;
     array_len[str_len].num_2++;
#endif

     if (str_num == 1) {
       build_single_str(t, str_len);
#if PROFILING
       type_num[SINGLE_STR].num++;
#endif
     } else {
       build_str_array(t, str_num, str_len);
#if PROFILING
       type_num[ARRAY].num++;
#endif
     }
}

