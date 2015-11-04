#ifndef HASH_H
#define HASH_H

#include "share.h"

typedef struct slot {
    Type_t type;
    Pat_Num_t item_num;
    void *collision_structure;
} Slot_t;

typedef struct  hast_table {
  char power;
  Pat_Num_t slots_num;
  Pat_Len_t lsp;
  Slot_t slots[];
} Hash_Table_t;

typedef struct collision_elmt {
  Char_t flag[1]; /* 第0位标示是否是模式尾, 第1位标示字符串是否内嵌 */
  union {
    Char_t buf[POINTER_SIZE];
    Char_t *p;
  } str;
  Expand_Node_t expand_node;
} Collision_Elmt_t;

void build_hash(Expand_Node_t *expand_node, Pat_Num_t dist_prf_num, Pat_Len_t lsp);
void print_hash(Hash_Table_t *hash_table);
inline unsigned hash(Char_t const *c, Pat_Len_t len, char power);
Expand_Node_t *match_hash(Hash_Table_t *hash_table, Char_t const **text, Bool_t *is_matched);

#endif 
