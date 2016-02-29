/* 120 lines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "common.h"
#include "binary.h"
#include "share.h"
#include "hash.h"
#include "queue.h"
#include "statistics.h"

extern Queue_t *queue;
extern Str_Num_t type_num[];
extern Str_Num_t fun_calls[];

static inline Hash_Value_t hash(Char_t const *s, Pat_Len_t len, Pat_Num_t table_size)
{
     register Hash_Value_t value = SEED;

     while (len--)
       value ^= (value << L_BITS) + (value >> R_BITS) + *((UC_t const *) s++);

     return value % table_size;
}

/* Hash表只能过滤一定不匹配的串,可能匹配的串需要由对应expand node的下一级来进一步判断 */
Expand_Node_t *match_hash(Hash_Table_t *hash_table, Char_t const **pos_p, Bool_t *is_pat_end)
{
#if PROFILING
  fun_calls[MATCH_HASH].num++;
#endif 

  Expand_Node_t *expand_node = hash_table->expand_nodes
    + hash(*pos_p, hash_table->str_len, hash_table->table_size);
  
  return expand_node->next_level == NULL ? NULL : expand_node;
}

static Hash_Table_t *make_hash_table(Pat_Num_t table_size, Pat_Len_t str_len)
{
     Hash_Table_t *new_hash_table = VMALLOC(Hash_Table_t, Expand_Node_t, table_size);

     new_hash_table->table_size = table_size;
     new_hash_table->str_len = str_len;
     memset(new_hash_table->expand_nodes, 0, sizeof(Expand_Node_t) * table_size); /* 初始化为0 */

     return new_hash_table;
}

/* 保存hash槽链表尾指针,用于维持槽链表有序 */
static Suffix_Node_t ***make_tails(Pat_Num_t table_size, Expand_Node_t const *expand_nodes)
{
  Suffix_Node_t ***tails = MALLOC(table_size, Suffix_Node_t **), ***p = tails;

  while (table_size--) 
    *p++ = (Suffix_Node_t **) &(expand_nodes++)->next_level;
  
  return tails;
}

void build_hash_table(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)  
{
  Hash_Table_t *hash_table;
  Suffix_Node_t *cur_suf, *next_suf, ***tails;
  Pat_Num_t table_size;
  Hash_Value_t hash_value;

#if PROFILING
  type_num[HASH].num++;
#endif

  table_size = ceill((long double) (str_num) / LOAD_FACTOR);
  hash_table = make_hash_table(table_size, str_len);
  tails = make_tails(table_size, hash_table->expand_nodes);

  /* 把每个后缀放入其对应的哈希槽中 */
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
    next_suf = cur_suf->next; cur_suf->next = NULL;
    hash_value = hash(cur_suf->str, str_len, table_size);
    *tails[hash_value] = cur_suf;
    tails[hash_value] = &cur_suf->next;
  }
  
  free(tails);
  
  expand_node->next_level = hash_table;
  expand_node->match_fun = (Match_Fun_t) match_hash;
  
  push_queue(hash_table->expand_nodes, table_size);
}
