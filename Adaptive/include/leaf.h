#ifndef LEAF_H
#define LEAF_H

#include "share.h"

typedef struct leaf {
     Pat_Num_t suf_num;
     Char_t str[];
} Leaf_t;

void print_leaf(Leaf_t *leaf);
void build_leaf(Expand_Node_t *expand_node);

#endif 
