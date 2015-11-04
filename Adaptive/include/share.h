#ifndef SHARE_H
#define SHARE_H


typedef unsigned char Char_t;
typedef unsigned char Pat_Len_t;
typedef unsigned Pat_Num_t;
typedef char Bool_t;
typedef char Type_t;


#define TYPE_NUM    8
#define LEAF        0
#define HASH        1
#define _4_MAP      2
#define _16_MAP     3
#define _48_MAP     4
#define _256_MAP    5
#define BST         6
#define _65536_MAP  7


#define PAT_NUM_THRESHOLD 100
#define TRUE 1
#define FALSE 0
#define MAX_PAT_LEN 256
#define POINTER_SIZE 8

typedef struct expand_node {
  Type_t type;
  void *next_level;
} Expand_Node_t;

typedef struct suffix_node {
  struct suffix_node *next;
  Char_t str[];
} Suffix_Node_t;

void get_num_and_lsp(Expand_Node_t const *expand_node, Pat_Num_t  *total_suf_p, Pat_Num_t *dif_prf_p, Pat_Len_t *lsp_p);
Suffix_Node_t *cut_head(Suffix_Node_t *suf_node, Pat_Len_t lsp);
void insert_to_expand(Expand_Node_t *expand_node, Suffix_Node_t *suf_node);
inline int same_str(Char_t const *s1, Char_t const *s2, Pat_Len_t len);
void print_str(char const *s, Pat_Len_t len, char terminator);

void print_suffix(Suffix_Node_t *cur_suf);

#endif
