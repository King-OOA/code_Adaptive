#pragma once

#include "share.h"

void build_array(Tree_Node_T t, Pat_Num_T str_num, Pat_Len_T str_len);
void build_single_str(Tree_Node_T t, Char_T *merged_pats);
void build_merged_str(Tree_Node_T t, Char_T *merged_pats, Pat_Len_T *len_array);
