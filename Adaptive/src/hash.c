/* 120 lines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "binary.h"
#include "share.h"
#include "statistics.h"

extern Str_Num_T type_num[];
extern Str_Num_T fun_calls[];

typedef struct Hash_Table {
     Pat_Num_T table_size;
     Pat_Len_T key_len;
     struct Tree_Node children[];
} *Hash_Table_T;

static inline Hash_Value_T hash(UC_T const *s, Pat_Len_T str_len, Pat_Num_T table_size)
{
     Hash_Value_T value = SEED;

     while (str_len--)
	  value ^= (value << L_BITS) + (value >> R_BITS) + *s++;

     return value % table_size;
}

/* Hash表只能过滤一定不匹配的串,可能匹配的串需要由对应expand node的下一级来进一步判断 */
Tree_Node_T match_hash(Tree_Node_T t, Char_T const *entrance, Char_T const **pos_p)
{
#if PROFILING
     fun_calls[MATCH_HASH].num++;
#endif
     
     Hash_Table_T hash_table = t->link;
     Tree_Node_T child = hash_table->children
	  + hash(*pos_p, hash_table->key_len, hash_table->table_size);
  
     return child->link == NULL ? NULL : child;
}

static Hash_Table_T hash_table_new(Pat_Num_T table_size, Pat_Len_T str_len)
{
     Hash_Table_T new_hash_table = VMALLOC(struct Hash_Table, struct Tree_Node, table_size);

     new_hash_table->table_size = table_size;
     new_hash_table->key_len = str_len;
     memset(new_hash_table->children, 0, sizeof(struct Tree_Node) * table_size); /* 初始化为0 */

     return new_hash_table;
}

/* 保存hash槽链表尾指针,用于维持槽链表有序 */
struct Suf_Node ***make_tails(Pat_Num_T table_size, Tree_Node_T child)
{
     struct Suf_Node ***tails = MALLOC(table_size, struct Suf_Node **), ***p = tails;

     while (table_size--) 
	  *p++ = (struct Suf_Node **) &(child++)->link;
  
     return tails;
}

void build_hash_table(Tree_Node_T t, Pat_Num_T str_num, Pat_Len_T str_len)  
{
#if PROFILING
     type_num[HASH].num++;
#endif

     Pat_Num_T table_size = ceill((long double) (str_num) / LOAD_FACTOR);
     Hash_Table_T hash_table = hash_table_new(table_size, str_len);
     struct Suf_Node ***tails = make_tails(table_size, hash_table->children);

     /* 把每个后缀放入其对应的哈希槽中 */
     for (Suf_Node_T cur_suf = t->link, next_suf; cur_suf; cur_suf = next_suf) {
	  next_suf = cur_suf->next;
	  cur_suf->next = NULL;
	  Hash_Value_T v = hash(cur_suf->str, str_len, table_size);
	  *tails[v] = cur_suf; tails[v] = &cur_suf->next;
     }
  
     free(tails);
  
     t->link = hash_table;
     t->match_fun = match_hash;
  
     push_children(hash_table->children, table_size);
}
