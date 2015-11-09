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

static unsigned power2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
			    8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576,
			    2097152, 4194304};

extern Queue_t *queue;
extern unsigned type_num[];


inline Hash_Value_t hash(Char_t const *c, Pat_Len_t len, char power)
{
     register Hash_Value_t value = SEED;

     while (len--)
       value ^= (value << L_BITS) + (value >> R_BITS) + *((UC_t const *) c++);
  
     return value & (1 << power) - 1;
}

Hash_Table_t *make_hash_table(Pat_Num_t slots_num, Pat_Len_t lsp, UC_t power)
{
     Hash_Table_t *new_hash_table = VMALLOC(Hash_Table_t, Expand_Node_t, slots_num);
     new_hash_table->slots_num = slots_num;
     new_hash_table->lsp = lsp;
     new_hash_table->power = power;
     memset(new_hash_table->slots, 0, sizeof(Expand_Node_t) * slots_num); /* slots初始化为0 */
     return new_hash_table;
}

/* 保存hash槽链表尾指针,用于维持槽链表有序 */
static Suffix_Node_t ***make_tail(Pat_Num_t slots_num, Expand_Node_t *slot)
{
  Suffix_Node_t ***tail = MALLOC(slots_num, Suffix_Node_t **), ***p = tail;

  while (slots_num--) 
    *p++ = (Suffix_Node_t **) &(slot++)->next_level;
  
  return tail;
}

void build_hash(Expand_Node_t *expand_node, Pat_Num_t dif_prf_num, Pat_Len_t lsp)  
{
  UC_t power;
  Hash_Table_t *hash_table;
  Suffix_Node_t *cur_suf, *next_suf;
  Pat_Num_t slots_num;
  Expand_Node_t *slot;
  unsigned hash_value;
  Suffix_Node_t ***tail;

  for (power = 1; dif_prf_num > power2[power]; power++)
    ;
     
  slots_num = power2[--power];
  hash_table = make_hash_table(slots_num, lsp, power);
  tail = make_tail(slots_num, hash_table->slots);
  
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /* 把所有后缀放入其对应的哈希槽中 */
    next_suf = cur_suf->next; cur_suf->next = NULL;
    hash_value = hash(cur_suf->str, lsp, power);
    *tail[hash_value] = cur_suf;
    tail[hash_value] = &cur_suf->next;
  }
	
  expand_node->next_level = hash_table;
  expand_node->type = HASH;
  type_num[HASH]++;
  free(tail);

  /* 将hash表新产生的expand_node加入队列 */
  for (slots_num = hash_table->slots_num, slot = hash_table->slots; slots_num; slot++, slots_num--)
    if (slot->next_level)
      in_queue(queue, slot);
}

/* Hash表只能确定一定不匹配的串,可能匹配的串需要由对应expand node的下一级来进一步判断 */
Expand_Node_t *match_hash(Hash_Table_t *hash_table, Char_t const **text, Bool_t *is_pat_end)
{
  Char_t const *s = *text;
  Expand_Node_t *slot = hash_table->slots + hash(s, hash_table->lsp, hash_table->power);
  
  return slot->next_level == NULL ? NULL : slot;
}

void print_hash(Hash_Table_t *hash_table)
{
    Expand_Node_t *slot = hash_table->slots;
    Pat_Num_t i;
    Pat_Num_t slots_num = hash_table->slots_num;
    Pat_Len_t lsp = hash_table->lsp;
     
    printf("Hashing@ slots number: %u, lsp: %u\n", slots_num, lsp);
    for (i = 0; i < slots_num; slot++, i++)
      if (slot->next_level) {
    	printf("slot num: %u\n", i);
    	print_suffix(slot->next_level);
      }
}

/* void build_hash(Expand_Node_t *expand_node, Pat_Num_t dif_prf_num, Pat_Len_t lsp)   */
/* { */
/*     char  power; */
/*     Hash_Table_t *hash_table; */
/*     Suffix_Node_t *cur_suf, *next_suf; */
/*     Char_t *suf_str; */
/*     Collision_Elmt_t *coli_elmt; */
/*     Pat_Num_t slots_num, item_num, *item_in_slot, i; */
/*     Slot_t *slot; */

/*     for (power = 1; dif_prf_num > power2[power]; power++) */
/*       ; */
     
/*     power--; */
/*     hash_table = make_hash_table(power2[power], lsp, power); */
     
/*     divide_to_slot(expand_node, hash_table, lsp, item_in_slot); /\* 先将所有后缀分配到指定哈希槽内,并统计冲突项数 *\/ */
    
/*     for (i = 0; i < hash_table->slots_num; i++) /\* 为每一个哈希槽建立冲突结构 *\/ */
/*       if (item_num = item_in_slot[i]) */
/* 	build_collision_structure(hash_table->slots + i, item_num); */
    
	

/* for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /\* 先把所有后缀放入其对应的哈希槽中 *\/ */
/*       next_suf = cur_suf->next; */
/*       suf_str = cur_suf->str; */
/*       slot = hash_table->slots + hash(suf_str, lsp, power); */
	  
/*       for (item_num = slot->item_num, coli_elmt = slot->collision_structure; */
/* 	   item_num && !same_str(suf_str, IS_IN_NODE(coli_elmt->flag) ? coli_elmt->str.buf : coli_elmt->str.p, lsp); */
/* 	   item_num--, coli_elmt++) */
/* 	; */
	 
/*       if (item_num == 0) {  /\* 没找到 *\/ */
/* 	coli_elmt = enlarge_coli_array(slot); /\* 新建一项,放于最后 *\/ */
/* 	if (lsp <= POINTER_SIZE) { /\* 放到节点内部 *\/ */
/* 	  memcpy(coli_elmt->str.buf, suf_str, lsp); */
/* 	  SET_IN_NODE(coli_elmt->flag); */
/* 	} else {  /\* 放到节点外部 *\/ */
/* 	  coli_elmt->str.p = MALLOC(lsp, Char_t); */
/* 	  memcpy(coli_elmt->str.p, suf_str, lsp); */
/* 	} */
/*       } */
	  
/*       if (cur_suf = cut_head(cur_suf, lsp)) */
/* 	insert_to_expand(&coli_elmt->expand_node, cur_suf); */
/*       else */
/* 	SET_PAT_END(coli_elmt->flag); */
/*     } */
  
/*     expand_node->next_level = hash_table; */
/*     expand_node->type = HASH; */
/*     type_num[HASH]++; */
  
/*     /\* 将hash表新产生的expand_node加入队列 *\/ */
/*     slots_num = hash_table->slots_num;  */
/*     for (slot = hash_table->slots; slots_num; slot++, slots_num--) */
/*       if (item_num = slot->item_num) */
/* 	for (coli_elmt = slot->collision_structure; item_num; coli_elmt++, item_num--) */
/* 	  if (coli_elmt->expand_node.next_level) */
/* 	    in_queue(queue, &coli_elmt->expand_node); */
/*   } */
