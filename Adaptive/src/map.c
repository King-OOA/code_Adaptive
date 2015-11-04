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


extern Queue_t *queue;
extern unsigned type_num[];


void build_4_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  Suffix_Node_t *cur_suf, *next_suf;
  _4_Map_t *_4_map;
  Char_t ch;
  Char_t *keys;
  Pat_Num_t i, j;

  _4_map = CALLOC(1, _4_Map_t);
  memset(_4_map->keys, UCHAR_MAX, 4);
  _4_map->dif_ch_num = dif_ch_num;
  keys = _4_map->keys;
  
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    ch = *cur_suf->str;
    for (i = 0; ch > keys[i]; i++)
      ;
    if (ch < keys[i]) {
      for (j = dif_ch_num - 2; j >= i; j--) /* 向后移动元素 */
	keys[j+1] = keys[j];
      keys[i] = ch; /* 插入元素 */
    }
    
    if (cur_suf = cut_head(cur_suf, 1))
      insert_to_expand(_4_map->expand_nodes + i, cur_suf);
    else
      set_bit(_4_map->pat_end_flag, i); /* 是模式尾 */
  }
 
  expand_node->next_level = _4_map;
  expand_node->type = _4_MAP;
  type_num[_4_MAP]++;
  
  for (i = 0; i < dif_ch_num; i++) /* 加入到队列 */
    if ((expand_node = _4_map->expand_nodes + i)->next_level)
      in_queue(queue, expand_node);
}

void build_16_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  Suffix_Node_t *cur_suf, *next_suf;
  _16_Map_t *_16_map;
  Char_t ch;
  Char_t *keys;
  Pat_Num_t i, j;

  _16_map = CALLOC(1, _16_Map_t);
  memset(_16_map->keys, UCHAR_MAX, 16);
  _16_map->dif_ch_num = dif_ch_num;
  keys = _16_map->keys;
  
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    ch = *cur_suf->str;
    for (i = 0; ch > keys[i]; i++)
      ;
    if (ch < keys[i]) {
      for (j = dif_ch_num - 2; j >= i; j--) /* 向后移动元素 */
	keys[j+1] = keys[j];
      keys[i] = ch; /* 插入元素 */
    }
    
    if (cur_suf = cut_head(cur_suf, 1))
      insert_to_expand(_16_map->expand_nodes + i, cur_suf);
    else
      set_bit(_16_map->pat_end_flag, i);
  }
 
  expand_node->next_level = _16_map;
  expand_node->type = _16_MAP;
  type_num[_16_MAP]++;
  
  for (i = 0; i < dif_ch_num; i++) /* 加入到队列 */
    if ((expand_node = _16_map->expand_nodes + i)->next_level)
      in_queue(queue, expand_node);
}

void build_48_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  _48_Map_t *_48_map = CALLOC(1, _48_Map_t);
  Suffix_Node_t *cur_suf, *next_suf;
  Char_t ch;
  int i = 0, index;
  
  memset(_48_map->index, -1, 256);

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    ch = *cur_suf->str;
    if (_48_map->index[ch] == -1)
      _48_map->index[ch] = i++;
    index = _48_map->index[ch];
    
    if (cur_suf = cut_head(cur_suf, 1))
      insert_to_expand(_48_map->expand_nodes + index, cur_suf);
    else  /* 模式串尾 */
      set_bit(_48_map->pat_end_flag, index);
  }

  assert(--i == dif_ch_num);
  
  expand_node->next_level = _48_map;
  expand_node->type = _48_MAP;
  type_num[_48_MAP]++;
  
  for (i = 0; i < dif_ch_num; i++)
    if (_48_map->expand_nodes[i].next_level)
      in_queue(queue, _48_map->expand_nodes + i);
}

void build_256_map(Expand_Node_t *expand_node)
{
  Suffix_Node_t *cur_suf, *next_suf;
  _256_Map_t *_256_map = CALLOC(1, _256_Map_t);
  Char_t ch;
  int n;

  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next;
    ch = *cur_suf->str;
    set_bit(_256_map->match_flag, ch); /* 标志匹配该字符 */
    if (cur_suf = cut_head(cur_suf, 1))
      insert_to_expand(_256_map->expand_nodes + ch, cur_suf);
    else
      set_bit(_256_map->pat_end_flag, ch);
  }
  
  expand_node->next_level = _256_map;
  expand_node->type = _256_MAP;
  type_num[_256_MAP]++;

  for (n = 256, expand_node = _256_map->expand_nodes; n; expand_node++, n--)
    if (expand_node->next_level)
      in_queue(queue, expand_node);
}

void build_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num)
{
  if (dif_ch_num <= 4)
    build_4_map(expand_node, dif_ch_num);
  else if (dif_ch_num <= 16)
    build_16_map(expand_node, dif_ch_num);
  else if (dif_ch_num <= 48)
    build_48_map(expand_node, dif_ch_num);
  else
    build_256_map(expand_node);
}

Expand_Node_t *match_4_map(_4_Map_t *_4_map, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t ch = **text, *key;
  Expand_Node_t *expand_node;
  char n, i;
  
  for (n = _4_map->dif_ch_num, key = _4_map->keys;
       n && ch > *key; n--, key++) /* key是有序排列的 */
    ;
  
  if (n == 0 || ch < *key) /* 匹配失败 */
    return NULL;

  i = key -_4_map->keys;
  expand_node = _4_map->expand_nodes + i;
  *is_pat_end = test_bit(_4_map->pat_end_flag, i);
  (*text)++;
  
  return expand_node;
}

Expand_Node_t *match_16_map(_16_Map_t *_16_map, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t *keys = _16_map->keys, ch = **text;
  Expand_Node_t *expand_node;
  char low = 0, high = _16_map->dif_ch_num - 1, mid;
  
  while (low <= high) {
    mid = (low + high) / 2;

    if (ch == keys[mid]) {
      expand_node = _16_map->expand_nodes + mid;
      *is_pat_end = test_bit(_16_map->pat_end_flag, mid);
      (*text)++;
      return expand_node;
    }  else if (ch < keys[mid])
      high = mid - 1;
    else
      low = mid + 1;
  }
  
  return NULL;
}

Expand_Node_t *match_48_map(_48_Map_t *_48_map, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t ch = **text;
  int i;
  
  if ((i = _48_map->index[ch]) == -1)
    return NULL;

  *is_pat_end = test_bit(_48_map->pat_end_flag, i);
  (*text)++;
  
  return _48_map->expand_nodes + i;
}

Expand_Node_t *match_256_map(_256_Map_t *_256_map, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t ch = **text;
  
  if (test_bit(_256_map->match_flag, ch) == 0)
    return NULL;
  
  *is_pat_end = test_bit(_256_map->pat_end_flag, ch);
  (*text)++;
  
  return _256_map->expand_nodes + ch;
}

void print_4_map(_4_Map_t *_4_map)
{
     int i;
     
     printf("\n4 map\n");
     for (i = 0; i < _4_map->dif_ch_num; i++) {
	  printf("\n%c:\n ", _4_map->keys[i]);
	  print_suffix(_4_map->expand_nodes[i].next_level);
     }
}

void print_16_map(_16_Map_t *_16_map)
{
     int i;
     
     printf("\n16 map\n");
     for (i = 0; i < _16_map->dif_ch_num; i++) {
	  printf("\n%c:\n ", _16_map->keys[i]);
	  print_suffix(_16_map->expand_nodes[i].next_level);
     }
}

void print_48_map(_48_Map_t *_48_map)
{
     int i;
     
     puts("\n48 map\n");
     for (i = 0; i < 256; i++)
	  if (_48_map->index[i] != -1) {
	       printf("\n%c:\n ", i);
	       print_suffix(_48_map->expand_nodes[_48_map->index[i]].next_level);
	  }
}

void print_256_map(_256_Map_t *_256_map)
{
     int i;
     
     puts("\n256 map");
     for (i = 0; i < 256; i++)
       if (test_bit(_256_map->match_flag, i)) {
	       printf("\n%c:\n ", i);
	       print_suffix(_256_map->expand_nodes[i].next_level);
	  }
}

