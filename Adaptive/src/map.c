#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "queue.h"
#include "share.h"
#include "binary.h"
#include "common.h"
#include "map.h"
#include "statistics.h"

extern Queue_T queue;
extern Num_Num_T map_size[];
extern Str_Num_T type_num[];
extern Str_Num_T fun_calls[];

/* 一定是终止节点 */
typedef struct Map_1 {
  struct Tree_Node child;
  Char_T key;
} *Map_1_T;

typedef struct Map_4 {
  struct Tree_Node children[4];
  Char_T keys[4];
  Flag_T pat_end_flag[1];
  uint8_t key_num; /* 最大为4 */
} *Map_4_T;

typedef struct Map_16 {
  struct Tree_Node children[16];
  Char_T keys[16];
  Flag_T pat_end_flag[16/8];
  uint8_t key_num; /* 最大为16 */
} *Map_16_T;

typedef struct Map_48 {
  struct Tree_Node children[48];
  int8_t index[256]; /* 非法索引值用-1表示 */
  Flag_T pat_end_flag[48/8];
} *Map_48_T;

typedef struct Map_256 {
  struct Tree_Node children[256];
  Flag_T pat_end_flag[256/8];
} *Map_256_T;

static Tree_Node_T match_map_1(Map_1_T map_1, Char_T const **pos_p, bool *find_pat_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_1].num++;
#endif

  if (**pos_p != map_1->key)
    return NULL;

  (*pos_p)++;
  *find_pat_p = true; /* 一定是某模式的终止节点 */

  return &map_1->child;
}

static Tree_Node_T match_map_4(Map_4_T map_4, Char_T const **pos_p, bool *find_pat_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_4].num++;
#endif

  Char_T t_ch = **pos_p, *keys = map_4->keys;	/* t_ch 为目标字符 */
  uint8_t i, key_num = map_4->key_num;

  /* key是有序排列的 */
  for (i = 0; i < key_num && keys[i] < t_ch; i++)
    ;

  if (i == key_num || keys[i] > t_ch) /* 匹配失败 */
    return NULL;
  
  /* 匹配成功 */
  *find_pat_p = test_bit(map_4->pat_end_flag, i);
  (*pos_p)++;

  return map_4->children + i;
}

static Tree_Node_T match_map_16(Map_16_T map_16, Char_T const **pos_p, bool *find_pat_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_16].num++;
#endif

  Char_T t_ch = **pos_p;
  int8_t low = 0, high = map_16->key_num - 1, mid; /* 必须是有符号数 */

  while (low <= high) {
    mid = (low + high) >> 1;
    Char_T key = map_16->keys[mid];
 
    if (t_ch < key)
     high = mid - 1;
    else if (t_ch > key)
      low = mid + 1;
    else {
      *find_pat_p = test_bit(map_16->pat_end_flag, mid);
      (*pos_p)++;
      return map_16->children + mid;
    }
  }

  return NULL;
}

static Tree_Node_T match_map_48(Map_48_T map_48, Char_T const **pos_p, bool *find_pat_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_48].num++;
#endif

  UC_T t_ch = **pos_p;
  int8_t i;

  if ((i = map_48->index[t_ch]) == -1)
    return NULL;

  *find_pat_p = test_bit(map_48->pat_end_flag, i);
  (*pos_p)++;

  return map_48->children + i;
}

static Tree_Node_T match_map_256(Map_256_T map_256, Char_T const **pos_p, bool *find_pat_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_256].num++;
#endif

  UC_T t_ch = **pos_p;

  *find_pat_p = test_bit(map_256->pat_end_flag, t_ch);
  /* 匹配的三种情况: 1.只是终止节点,没有后续; 2.既是终止节点,又有后续; 3.不是终止节点,但有后续 */
  Tree_Node_T child = map_256->children + t_ch;
  if (*find_pat_p || child->link)
    (*pos_p)++;

  return child->link == NULL ? NULL : child;
}

/* 所有后缀的首字符相同 */
void build_map_1(Tree_Node_T t)
{
  Map_1_T map_1 = CALLOC(1, struct Map_1);
  map_1->key = *((Suf_Node_T) t->link)->str;

#if DEBUG
  assert(map_1->key);
#endif 

  struct Suf_Node **next_p = (struct Suf_Node **) &map_1->child.link;
 
  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;	/* 必须判断cur_suf非空时才能指向其下一个 */
#if DEBUG
    assert(*cur_suf->str == map_1->key);
#endif
    if ((cur_suf = cut_head(cur_suf, 1))) { /* 如果当前后缀去掉首字符后还存在 */
      *next_p = cur_suf; next_p = &cur_suf->next; /* 则将去掉首字符的后缀,插入到链表中 */
    }
  }

  *next_p = NULL;

  t->match_fun = (Match_Fun_T) match_map_1;
  t->link = map_1;

  push_queue(&map_1->child, 1);
}

#define BUILD_MAP_4_OR_16(n)						\
									\
static void build_map_##n(Tree_Node_T t, Pat_Num_T key_num)		\
{									\
  Map_##n##_T map_##n = CALLOC(1, struct Map_##n);			\
  map_##n->key_num = key_num;						\
									\
  struct Suf_Node **next_p = (struct Suf_Node **) &map_##n->children[0]; /* 初始化 */ \
  Char_T cur_ch, pre_ch = 0;						\
  int8_t i = -1; /* 索引初始值 */					\
  /*具有相同首字符的后缀,连接在一起,形成段,每段对应keys中的一个字符*/	\
  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) { \
    next_suf = cur_suf->next;						\
   /*判断是否是新一段的开始*/						\
    if ((cur_ch = *cur_suf->str) != pre_ch) {				\
      *next_p = NULL; map_##n->keys[++i] = cur_ch;  /* 当前链表截止 */	\
      next_p = (struct Suf_Node **) &map_##n->children[i]; /*新一段的链表头 */ \
      pre_ch = cur_ch;							\
    }									\
    /* 将当前后缀插入到对应孩子节点中 */				\
    if ((cur_suf = cut_head(cur_suf, 1))) {				\
      *next_p = cur_suf; next_p = &cur_suf->next;			\
    } else								\
      set_bit(map_##n->pat_end_flag, i); /* 是模式尾 */			\
  }									\
									\
  *next_p = NULL;							\
									\
  t->link = map_##n;							\
  t->match_fun = (Match_Fun_T) match_map_##n;				\
  									\
  push_queue(map_##n->children, key_num);				\
}

BUILD_MAP_4_OR_16(4)

BUILD_MAP_4_OR_16(16)

static void build_map_48(Tree_Node_T t, Pat_Num_T key_num)
{
  Map_48_T map_48 = CALLOC(1, struct Map_48);
  memset(map_48->index, -1, 256);

  struct Suf_Node **next_p = (struct Suf_Node **) &map_48->children[0]; /* 初始化 */
  UC_T cur_ch, pre_ch = 0;
  int8_t i = -1; /* 0 ~ 47 */

  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG
    assert(*cur_suf->str);
#endif
   /* 是否是新一段开始 */
    if ((cur_ch = *cur_suf->str) != pre_ch) {
#if DEBUG     
    assert(cur_ch > pre_ch);
#endif 
     *next_p = NULL; map_48->index[cur_ch] = ++i;
      next_p = (struct Suf_Node **) &map_48->children[i];
      pre_ch = cur_ch;
    }

    if ((cur_suf = cut_head(cur_suf, 1))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else  /* 模式串尾 */
      set_bit(map_48->pat_end_flag, i);
  }

  *next_p = NULL;

#if DEBUG
  assert(key_num == i+1);
#endif 

  t->link = map_48;
  t->match_fun = (Match_Fun_T) match_map_48;

  push_queue(map_48->children, key_num);
}

static void build_map_256(Tree_Node_T t)
{
  Map_256_T map_256 = CALLOC(1, struct Map_256);

  struct Suf_Node **next_p = (struct Suf_Node **) &map_256->children[0]; /* 初始化 */
  UC_T pre_ch = 0, cur_ch;

  for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG     
    assert(*cur_suf->str);
#endif 
    if ((cur_ch = *cur_suf->str) != pre_ch) { /* 终止上一个链表,跳转到新链表头 */
#if DEBUG     
    assert(cur_ch > pre_ch);
#endif 
      *next_p = NULL;
      next_p = (struct Suf_Node **) &map_256->children[cur_ch];
      pre_ch = cur_ch;
    }

    if ((cur_suf = cut_head(cur_suf, 1))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(map_256->pat_end_flag, cur_ch);
  }

  *next_p = NULL;

  t->link = map_256;
  t->match_fun = (Match_Fun_T) match_map_256;

  push_queue(map_256->children, 256);
}

void build_map(Tree_Node_T t, Pat_Num_T key_num)
{
#if PROFILING
  map_size[key_num].num_1 = key_num;
  map_size[key_num].num_2++;
#endif

  if (key_num == 1) {	/* 单个字符 */
    build_map_1(t);
#if PROFILING
    type_num[MAP_1].num++;
#endif
  } else if (key_num <= 4) {
    build_map_4(t, key_num);
#if PROFILING
    type_num[MAP_4].num++;
#endif
  } else if (key_num <= 16) {
    build_map_16(t, key_num);
#if PROFILING
    type_num[MAP_16].num++;
#endif
  } else if (key_num <= 48) {
    build_map_48(t, key_num);
#if PROFILING
    type_num[MAP_48].num++;
#endif
  } else {
    build_map_256(t);
#if PROFILING
    type_num[MAP_256].num++;
#endif
  }
}
