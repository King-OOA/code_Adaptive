#ifndef SHARE_H
#define SHARE_H

typedef unsigned char UC_t;
typedef char Char_t;
typedef UC_t Pat_Len_t;
typedef unsigned Pat_Num_t;
typedef char Bool_t;
typedef UC_t Type_t;
typedef UC_t Flag_t;
typedef unsigned Hash_Value_t;

#define TYPE_NUM    8
#define END         0
#define ARRAY       1
#define HASH        2
#define _4_MAP      3
#define _16_MAP     4
#define _48_MAP     5
#define _256_MAP    6
#define _65536_MAP  7


#define PAT_NUM_THRESHOLD 100
#define TRUE 1
#define FALSE 0
#define MAX_PAT_LEN 256
#define POINTER_SIZE 8
#define SMALL_ARRAY_SIZE 20


typedef struct expand_node {
  Type_t type;
  void *next_level;
} Expand_Node_t;

typedef struct suffix_node {
  struct suffix_node *next;
  Char_t str[];
} Suffix_Node_t;

typedef Suffix_Node_t  List_Node_t;

typedef struct str_elmt {
  Flag_t pat_end_flag; /* 第0位标示是否是模式尾, 第1位标示字符串是否内嵌 */
  union {
    Char_t buf[POINTER_SIZE];
    Char_t *p;
  } str;
  Expand_Node_t expand_node;
} Str_Elmt_t;


void get_num_and_lsp(Expand_Node_t const *expand_node, Pat_Num_t  *total_suf_p, Pat_Num_t *dif_prf_p, Pat_Len_t *lsp_p);
Suffix_Node_t *cut_head(Suffix_Node_t *suf_node, Pat_Len_t lsp);
void insert_to_expand(Expand_Node_t *expand_node, Suffix_Node_t *suf_node);
inline Bool_t same_str(Char_t const *s1, Char_t const *s2, Pat_Len_t len);
inline int str_n_cmp(Char_t const *s1, Char_t const *s2, Pat_Len_t len);
void print_str(Char_t const *s, Pat_Len_t len, Char_t terminator);
List_Node_t * list_radix_sort(List_Node_t *list_head);
void remove_duplicate(Suffix_Node_t *suf_list);

void print_suffix(Suffix_Node_t *cur_suf);

#endif
