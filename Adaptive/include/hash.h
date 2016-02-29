#ifndef HASH_H
#define HASH_H

#include "share.h"

typedef struct  hast_table {
  Pat_Num_t table_size;
  Pat_Len_t str_len;
  Expand_Node_t expand_nodes[];
} Hash_Table_t;

void build_hash_table(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len);

#endif 
