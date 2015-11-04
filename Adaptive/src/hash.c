#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "common.h"
#include "binary.h"
#include "share.h"
#include "hash.h"
#include "queue.h"

#define L_BITS 6
#define R_BITS 2
#define SEED 50u

#define IS_IN_NODE(flag) test_bit((flag), 1)
#define SET_IN_NODE(flag) set_bit((flag), 1) 

#define IS_PAT_END(flag) test_bit((flag), 0)
#define SET_PAT_END(flag) set_bit((flag), 0) 
     
static unsigned power2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
			    8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576,
			    2097152, 4194304};

extern Queue_t *queue;
extern unsigned type_num[];

inline unsigned hash(Char_t const *c, Pat_Len_t len, char power)
{
     register unsigned value = SEED;

     while (len--)
	  value ^= (value << L_BITS) + (value >> R_BITS) + *c++;
  
     return value & (1 << power) - 1;
}

Hash_Table_t *make_hash_table(Pat_Num_t slots_num, Pat_Len_t lsp, char power)
{
     Hash_Table_t *new_hash_table = VMALLOC(Hash_Table_t, Slot_t, slots_num);
     new_hash_table->slots_num = slots_num;
     new_hash_table->lsp = lsp;
     new_hash_table->power = power;
     memset(new_hash_table->slots, 0, sizeof(Slot_t) * slots_num); /* slots初始化为0 */
     return new_hash_table;
}

static Collision_Elmt_t *enlarge_coli_array(Slot_t *slot)
{
     Collision_Elmt_t *new_coli_elmt;
     
     if ((slot->collision_structure = /* 在原有的基础上新分配一个 */
	  realloc(slot->collision_structure, ++slot->item_num * sizeof(Collision_Elmt_t))) == NULL) {
	  fprintf(stderr, "Realloc failed!\n");
	  exit(EXIT_FAILURE);
     }
     
     new_coli_elmt = (Collision_Elmt_t *) slot->collision_structure + slot->item_num - 1; /* 指向新分配的冲突项 */
     new_coli_elmt->flag[0] = 0u;	/* 默认字符串放于节点外,也不是终止节点 */
     new_coli_elmt->expand_node.type = 0;
     new_coli_elmt->expand_node.next_level = NULL;
     
     return new_coli_elmt;
}

void build_hash(Expand_Node_t *expand_node, Pat_Num_t dif_prf_num, Pat_Len_t lsp)  
{
     char  power;
     Hash_Table_t *hash_table;
     Suffix_Node_t *cur_suf, *next_suf;
     Char_t *suf_str;
     Collision_Elmt_t *coli_elmt;
     Pat_Num_t slots_num, item_num;
     Slot_t *slot;

     for (power = 1; dif_prf_num > power2[power]; power++)
	  ;
     
     power--;
     hash_table = make_hash_table(power2[power], lsp, power);
  
     for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
	  next_suf = cur_suf->next;
	  suf_str = cur_suf->str;
	  slot = hash_table->slots + hash(suf_str, lsp, power);
	  
	  for (item_num = slot->item_num, coli_elmt = slot->collision_structure;
	       item_num && !same_str(suf_str, IS_IN_NODE(coli_elmt->flag) ? coli_elmt->str.buf : coli_elmt->str.p, lsp);
	       item_num--, coli_elmt++)
	       ;
	 
	  if (item_num == 0) {  /* 没找到 */
	       coli_elmt = enlarge_coli_array(slot); /* 新建一项,放于最后 */
	       if (lsp <= POINTER_SIZE) { /* 放到节点内部 */
		    memcpy(coli_elmt->str.buf, suf_str, lsp);
		    SET_IN_NODE(coli_elmt->flag);
	       } else {  /* 放到节点外部 */
		    coli_elmt->str.p = MALLOC(lsp, Char_t);
		    memcpy(coli_elmt->str.p, suf_str, lsp);
	       }
	  }
	  
	  if (cur_suf = cut_head(cur_suf, lsp))
	       insert_to_expand(&coli_elmt->expand_node, cur_suf);
	  else
	       SET_PAT_END(coli_elmt->flag);
     }
  
     expand_node->next_level = hash_table;
     expand_node->type = HASH;
     type_num[HASH]++;
  
     /* 将hash表新产生的expand_node加入队列 */
     slots_num = hash_table->slots_num; 
     for (slot = hash_table->slots; slots_num; slot++, slots_num--)
	  if (item_num = slot->item_num)
	       for (coli_elmt = slot->collision_structure; item_num; coli_elmt++, item_num--)
		    if (coli_elmt->expand_node.next_level)
			 in_queue(queue, &coli_elmt->expand_node);
}

Expand_Node_t *match_hash(Hash_Table_t *hash_table, Char_t const **text, Bool_t *is_pat_end)
{
     Char_t const *s = *text;
     Collision_Elmt_t *coli_elmt;
     Pat_Num_t item_num;
     Pat_Len_t lsp = hash_table->lsp;
  
     Slot_t *slot = hash_table->slots + hash(s, lsp, hash_table->power);
  
     if (slot->item_num == 0) /* hash时便没找到 */
	  return NULL;

     for (item_num = slot->item_num, coli_elmt = slot->collision_structure; /* 遍历冲突表 */
	  item_num && !same_str(s, IS_IN_NODE(coli_elmt->flag) ? coli_elmt->str.buf : coli_elmt->str.p, lsp);
	  item_num--, coli_elmt++)
	  ;
  
     if (item_num == 0) /* 不在冲突表中 */
	  return NULL;
  
     *text += lsp; /* 文本指针前移 */
     *is_pat_end = IS_PAT_END(coli_elmt->flag);
  
     return &coli_elmt->expand_node;
}

void print_hash(Hash_Table_t *hash_table)
{
     Slot_t *slot = hash_table->slots;
     Pat_Num_t i, j;
     Pat_Num_t slots_num = hash_table->slots_num, item_num;
     Pat_Len_t lsp = hash_table->lsp;
     Collision_Elmt_t *coli_array;
     Char_t *sub_pat;
     Suffix_Node_t *cur_suf;
     
     printf("Hashing@ slots number: %u, lsp: %u\n", slots_num, lsp);
     for (i = 0; i < slots_num; i++)
	  if (item_num = slot[i].item_num) {
	       coli_array = slot[i].collision_structure;
	       printf("\nslot %d :\n", i);
	       for (j = 0; j < item_num; j++) {
		    sub_pat = IS_IN_NODE(coli_array[j].flag) ? coli_array[j].str.buf : coli_array[j].str.p;
		    print_str(sub_pat, lsp, ':');
		    for (cur_suf = coli_array[j].expand_node.next_level;
			 cur_suf;
			 cur_suf = cur_suf->next)
			 printf(" %s", cur_suf->str);
		    putchar('\n');
	       }
	  }
}
