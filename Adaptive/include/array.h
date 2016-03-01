#ifndef ARRAY_H
#define ARRAY_H

#include "share.h"

typedef struct single_str_t {
  Expand_Node_t expand_node[1];
  Pat_Len_t str_len;
  Char_t str[];
} Single_Str_t;

typedef struct str_array {
  Pat_Num_t str_num;
  Pat_Len_t str_len;
  Expand_Node_t *expand_nodes;	/* 外置扩展节点数组 */
  union {
    Flag_t flag[POINTER_SIZE];
    Flag_t *p;
  } pat_end_flag;
  Char_t str_buf[];
} Str_Array_t;

void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len);

#endif 
