#ifndef MAP_H
#define MAP_H

#include "share.h"

/* 一定是终止节点 */
typedef struct single_ch {
  Expand_Node_t expand_node[1];
  Char_t ch;
} Single_Ch_t;

typedef struct map_4 {
  Expand_Node_t expand_nodes[4];
  Char_t keys[4];
  Flag_t pat_end_flag[1];
  char ch_num; /* 最大为4 */
} Map_4_t;

typedef struct map_16 {
  Expand_Node_t expand_nodes[16];
  Char_t keys[16];
  Flag_t pat_end_flag[16/8];
  char ch_num; /* 最大为16 */
} Map_16_t;

typedef struct map_48 {
  Expand_Node_t expand_nodes[48];
  char index[256]; /* 非法索引值用-1表示 */
  Flag_t pat_end_flag[48/8];
} Map_48_t;

typedef struct map_256 {
  Expand_Node_t expand_nodes[256];
  Flag_t pat_end_flag[256/8];
} Map_256_t;

void build_map(Expand_Node_t *expand_node, Pat_Num_t ch_num);

#endif 
