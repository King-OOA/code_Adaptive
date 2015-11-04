#ifndef MAP_H
#define MAP_H

#include "share.h"

typedef struct _4_map {
  char dif_ch_num; /* 最大为4 */
  Char_t keys[4];
  Expand_Node_t expand_nodes[4];
  Char_t pat_end_flag[1];
} _4_Map_t;

typedef struct _16_map {
  char dif_ch_num; /* 最大为16 */
  Char_t keys[16];
  Expand_Node_t expand_nodes[16];
  Char_t pat_end_flag[16/8];
} _16_Map_t;

typedef struct _48_map {
  char index[256]; /* 非法索引值用-1表示 */
  Expand_Node_t expand_nodes[48];
  Char_t pat_end_flag[48/8];
} _48_Map_t;

typedef struct _256_map{
  Char_t pat_end_flag[256/8];
  Char_t match_flag[256/8];
  Expand_Node_t expand_nodes[256];
} _256_Map_t;

void build_4_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num);
void build_16_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num);
void build_48_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num);
void build_256_map(Expand_Node_t *expand_node);
void build_map(Expand_Node_t *expand_node, Pat_Num_t dif_ch_num);

Expand_Node_t *match_4_map(_4_Map_t *_4_map, Char_t const **text, Bool_t *is_pat_end);
Expand_Node_t *match_16_map(_16_Map_t *_16_map, Char_t const **text, Bool_t *is_pat_end);
Expand_Node_t *match_48_map(_48_Map_t *_48_map, Char_t const **text, Bool_t *is_pat_end);
Expand_Node_t *match_256_map(_256_Map_t *_256_map, Char_t const **text, Bool_t *is_pat_end);

void print_4_map(_4_Map_t *_4_map);
void print_16_map(_16_Map_t *_16_map);
void print_48_map(_48_Map_t *_48_map);
void print_256_map(_256_Map_t *_256_map);

#endif 
