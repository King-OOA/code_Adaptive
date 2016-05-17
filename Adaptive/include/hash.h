#pragma once

#include "share.h"

void build_hash_table(Tree_Node_T t, Pat_Num_T str_num, Pat_Len_T str_len);
struct Suf_Node ***make_tails(Pat_Num_T table_size, Tree_Node_T child);
