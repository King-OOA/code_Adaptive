#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "hash.h"
#include "share.h"
#include "queue.h"
#include "leaf.h"
#include "map.h"


Queue_t *queue;
unsigned type_num[TYPE_NUM];
const char *type_name[] = {"leaf", "hash", "4 map", "16 map", "48 map", "256 map", "BST", "65536 map"};

Suffix_Node_t *make_suffix_node(char const *pat)
{
     Suffix_Node_t *new_suf_node;
     
     new_suf_node = VMALLOC(Suffix_Node_t, char, strlen(pat) + 1);
     new_suf_node->next = NULL;
     strcpy(new_suf_node->str, pat);
     
     return new_suf_node;
}

static Suffix_Node_t *read_pats(FILE *pats_fp)
{
     char buf[MAX_PAT_LEN+1]; /*模式串缓存，最大1000个字符，包括换行符*/
     int pat_len;
     char *line_break = NULL; /*换行符指针*/
     Suffix_Node_t *list_head = NULL, *new_suf_node = NULL; /* 链表头指针 */
     
     while (fgets(buf, sizeof(buf), pats_fp)) {
	  if (line_break = strchr(buf, '\n'))
	       *line_break = '\0';
	  if (pat_len = strlen(buf)) {
	       new_suf_node = make_suffix_node(buf);
	       new_suf_node->next = list_head;
	       list_head = new_suf_node;
	  }
     }
  
     return list_head;
}

static Expand_Node_t *make_root(FILE *pats_fp)
{
     Suffix_Node_t *pat_list, *cur_suf, *next_suf;
     Expand_Node_t *root = CALLOC(1, Expand_Node_t);
     
     pat_list = read_pats(pats_fp);
     
     for (cur_suf = pat_list; cur_suf; cur_suf = next_suf) {
	  next_suf = cur_suf->next;
	  insert_to_expand(root, cur_suf, 0);
     }
     
     return root;
}

void choose_adaptor(Expand_Node_t *expand_node)
{
  unsigned total_suf_num, dif_prf_num, lsp; 	/* 最短模式串长 */
     
  get_num_and_lsp(expand_node, &total_suf_num, &dif_prf_num, &lsp);

  if (total_suf_num <= 5)
    build_leaf(expand_node);
  else {
    if (lsp == 1)
      build_map(expand_node, dif_prf_num);
    else if (dif_prf_num >= 5)
      build_hash(expand_node, total_suf_num, lsp);
  }
  
}

void print_expand(Expand_Node_t *expand_node)
{
  switch (expand_node->type) {
  case HASH: print_hash(expand_node->next_level);break;
  case _4_MAP: print_4_map(expand_node->next_level);break;
  case _16_MAP: print_16_map(expand_node->next_level);break;
  case _48_MAP: print_48_map(expand_node->next_level);break;
  case _256_MAP: print_256_map(expand_node->next_level);break;
  case LEAF : print_leaf(expand_node->next_level); break;
  }
  
}

void match_round(Expand_Node_t *expand_node, char const *text)
{
     Expand_Node_t *expand_node;
     char is_matched = 0;
     
     while (expand_node) {
	  expand_node = 
     }

}


int main(int argc, char **argv)
{
     FILE *pats_fp;
     Expand_Node_t *root, *expand_node;
     int i;
     
     queue = make_queue();
     
     pats_fp = Fopen(argv[1], "r");
     root = make_root(pats_fp);
     Fclose(pats_fp);
     
     in_queue(queue, root);

     while (!queue_is_empty(queue)) {
	  expand_node = out_queue(queue);
	  choose_adaptor(expand_node);
     }
     
     free_queue(queue);
     
     for (i = 0; i < TYPE_NUM; i++)
	  if (type_num[i])
	       printf("%s: %u\n", type_name[i], type_num[i]);
     
     //     print_leaf(root->next_level);
     
//     choose_adaptor(root);
//     print_expand(root);
     
}


