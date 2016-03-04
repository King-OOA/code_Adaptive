#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "share.h"
#include "binary.h"
#include "common.h"
#include "map.h"
#include "statistics.h"

extern Queue_t *queue;
extern Num_Num_t map_size[];
extern Str_Num_t type_num[];
extern Str_Num_t fun_calls[];

static Expand_Node_t *match_single_ch(Single_Ch_t *single_ch, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_SINGLE_CH].num++;
#endif

  if (**pos_p != single_ch->ch)
    return NULL;

  (*pos_p)++;
  *is_pat_end_p = TRUE;

  return single_ch->expand_node;
}

static Expand_Node_t *match_map_4(Map_4_t *map_4, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_4].num++;
#endif

  Char_t t_ch = **pos_p, *key;	/* t_ch 为目标字符 */
  char ch_num, i;

  /* key是有序排列的 */
  for (ch_num = map_4->ch_num, key = map_4->keys; ch_num && *key < t_ch; ch_num--, key++)
    ;

  if (ch_num == 0 || *key > t_ch) /* 匹配失败 */
    return NULL;
  
  /* 匹配成功 */
  i = key - map_4->keys;
  *is_pat_end_p = test_bit(map_4->pat_end_flag, i);
  (*pos_p)++;

  return map_4->expand_nodes + i;
}

static Expand_Node_t *match_map_16(Map_16_t *map_16, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_16].num++;
#endif

  Char_t t_ch = **pos_p, key;
  char low = 0, high = map_16->ch_num - 1, mid; /* 必须是有符号数 */

  while (low <= high) {
    mid = (low + high) >> 1;
    key = map_16->keys[mid];
 
   if (t_ch < key)
      high = mid - 1;
    else if (t_ch > key)
      low = mid + 1;
    else {
      *is_pat_end_p = test_bit(map_16->pat_end_flag, mid);
      (*pos_p)++;
      return map_16->expand_nodes + mid;
    }
  }

  return NULL;
}

static Expand_Node_t *match_map_48(Map_48_t *map_48, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_48].num++;
#endif

  UC_t t_ch = **pos_p;
  int i;

  if ((i = map_48->index[t_ch]) == -1)
    return NULL;

  *is_pat_end_p = test_bit(map_48->pat_end_flag, i);
  (*pos_p)++;

  return map_48->expand_nodes + i;
}

static Expand_Node_t *match_map_256(Map_256_t *map_256, Char_t const **pos_p, Bool_t *is_pat_end_p)
{
#if PROFILING
  fun_calls[MATCH_MAP_256].num++;
#endif

  UC_t t_ch = **pos_p;
  Expand_Node_t *expand_node = map_256->expand_nodes + t_ch;

  *is_pat_end_p = test_bit(map_256->pat_end_flag, t_ch);
  /* 三种情况: 1.只是终止节点,没有后续; 2.既是终止节点,又有后续; 3.不是终止节点,但有后续 */
  if (*is_pat_end_p || expand_node->next_level)
    (*pos_p)++;

  return expand_node;
}

/* 所有后缀的首字符相同 */
void build_single_ch(Expand_Node_t *expand_node)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Single_Ch_t *single_ch = CALLOC(1, Single_Ch_t);

  cur_suf = expand_node->next_level;
  single_ch->ch = *cur_suf->str;

#if DEBUG
  assert(single_ch->ch);
#endif 

  next_p = (Suffix_Node_t **) &single_ch->expand_node->next_level;
 
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG
    assert(*cur_suf->str == single_ch->ch);
#endif
    if ((cur_suf = cut_head(cur_suf, 1))) { /* 如果当前后缀去掉首字符后还存在 */
      *next_p = cur_suf; next_p = &cur_suf->next; /* 则将去掉首字符的后缀,插入到链表中 */
    } 
  }

  *next_p = NULL;

  expand_node->next_level = single_ch;
  expand_node->match_fun = (Match_Fun_t) match_single_ch;

  push_queue(single_ch->expand_node, 1);
}

#define BUILD_MAP_4_OR_16(n)						\
									\
static void build_map_##n(Expand_Node_t *expand_node, Pat_Num_t ch_num) \
{									\
  Suffix_Node_t *cur_suf, *next_suf, **next_p;				\
  Map_##n##_t *map_##n;							\
  Expand_Node_t *expand_nodes_##n;					\
  Char_t *keys, cur_ch, pre_ch = 0;					\
  int i = -1; /* 索引初始值 */						\
									\
  map_##n = CALLOC(1, Map_##n##_t);					\
  map_##n->ch_num = ch_num;						\
  keys = map_##n->keys;							\
  expand_nodes_##n = map_##n->expand_nodes;				\
  next_p = (Suffix_Node_t **) &expand_nodes_##n[0].next_level; /* 初始化 */ \
  /*具有相同首字符的后缀,连接在一起,形成段,每段对应keys中的一个字符*/	\
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { \
    next_suf = cur_suf->next;						\
    assert(*cur_suf->str);						\
   /*判断是否是新一段的开始*/						\
    if ((cur_ch = *cur_suf->str) != pre_ch) {				\
      *next_p = NULL; keys[++i] = cur_ch;  /* 当前链表截止 */		\
      next_p = (Suffix_Node_t **) &expand_nodes_##n[i].next_level; /*新一段的链表头 */ \
      pre_ch = cur_ch;							\
    }									\
    /* 将当前后缀插入到对应exp_node链表 */				\
    if ((cur_suf = cut_head(cur_suf, 1))) {				\
      *next_p = cur_suf; next_p = &cur_suf->next;			\
    } else								\
      set_bit(map_##n->pat_end_flag, i); /* 是模式尾 */			\
  }									\
									\
  *next_p = NULL;							\
									\
  expand_node->next_level = map_##n;					\
  expand_node->match_fun = (Match_Fun_t) match_map_##n;			\
  									\
  push_queue(map_##n->expand_nodes, ch_num);				\
}

BUILD_MAP_4_OR_16(4)

BUILD_MAP_4_OR_16(16)

static void build_map_48(Expand_Node_t *expand_node, Pat_Num_t ch_num)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Map_48_t *map_48 = CALLOC(1, Map_48_t);
  Expand_Node_t *expand_nodes_48 = map_48->expand_nodes;
  char *index = map_48->index;
  UC_t cur_ch, pre_ch = 0;
  int i = -1;

  memset(index, -1, 256);
  next_p = (Suffix_Node_t **) &expand_nodes_48[0].next_level; /* 随便初始化 */

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG
    assert(*cur_suf->str);
#endif
   /* 是否是新一段开始 */
    if ((cur_ch = *cur_suf->str) != pre_ch) {
      *next_p = NULL; index[cur_ch] = ++i;
      next_p = (Suffix_Node_t **) &expand_nodes_48[i].next_level;
      pre_ch = cur_ch;
    }

    if ((cur_suf = cut_head(cur_suf, 1))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else  /* 模式串尾 */
      set_bit(map_48->pat_end_flag, i);
  }

  *next_p = NULL;

#if DEBUG
  assert(ch_num == i+1);
#endif 

  expand_node->next_level = map_48;
  expand_node->match_fun = (Match_Fun_t) match_map_48;

  push_queue(map_48->expand_nodes, ch_num);
}

static void build_map_256(Expand_Node_t *expand_node)
{
  Suffix_Node_t *cur_suf, *next_suf, **next_p;
  Map_256_t *map_256 = CALLOC(1, Map_256_t);
  Expand_Node_t *expand_nodes_256 = map_256->expand_nodes;
  UC_t pre_ch = 0, cur_ch;

  next_p = (Suffix_Node_t **) &expand_nodes_256[0].next_level; /* 初始化 */

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
#if DEBUG     
    assert(*cur_suf->str);
#endif 
    if ((cur_ch = *cur_suf->str) != pre_ch) { /* 终止上一个链表,跳转到新链表头 */
      *next_p = NULL;
      next_p = (Suffix_Node_t **) &expand_nodes_256[cur_ch].next_level;
      pre_ch = cur_ch;
    }

    if ((cur_suf = cut_head(cur_suf, 1))) {
      *next_p = cur_suf; next_p = &cur_suf->next;
    } else
      set_bit(map_256->pat_end_flag, cur_ch);
  }

  *next_p = NULL;

  expand_node->next_level = map_256;
  expand_node->match_fun = (Match_Fun_t) match_map_256;

  push_queue(map_256->expand_nodes, 256);
}

void build_map(Expand_Node_t *expand_node, Pat_Num_t ch_num)
{
#if PROFILING
  map_size[ch_num].num_1 = ch_num;
  map_size[ch_num].num_2++;
#endif

  if (ch_num == 1) {	/* 单个字符 */
    build_single_ch(expand_node);
#if PROFILING
    type_num[SINGLE_CH].num++;
#endif
  } else if (ch_num <= 4) {
    build_map_4(expand_node, ch_num);
#if PROFILING
    type_num[MAP_4].num++;
#endif
  } else if (ch_num <= 16) {
    //    printf("%d\n", ++counter);
    build_map_16(expand_node, ch_num);
#if PROFILING
    type_num[MAP_16].num++;
#endif
  } else if (ch_num <= 48) {
    build_map_48(expand_node, ch_num);
#if PROFILING
    type_num[MAP_48].num++;
#endif
  } else {
    build_map_256(expand_node);
#if PROFILING
    type_num[MAP_256].num++;
#endif
  }
}
