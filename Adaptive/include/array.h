#ifndef LEAF_H
#define LEAF_H

#include "share.h"

typedef struct str_array {
  Pat_Num_t str_num;
  Pat_Len_t str_len;
  Str_Elmt_t array[];
} Str_Array_t;


void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len);
void build_array_1(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len);
Expand_Node_t *match_array(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end);
Expand_Node_t *match_array_2(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end);
Str_Array_t *make_array(Pat_Num_t str_num, Pat_Len_t str_len);
Expand_Node_t *match_array_1(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end);
void print_array(Str_Array_t *str_array);


#endif 
