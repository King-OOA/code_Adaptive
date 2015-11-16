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

#define TYPE_NUM    10
#define END         0
#define SINGLE_CH   1
#define MAP_4       2
#define MAP_16      3
#define MAP_48      4
#define MAP_256     5
#define SINGLE_STR  6
#define ARRAY       7
#define BINARY      8 /* 数组二分查找 */
#define HASH        9

#define NUM_TO_BUILD_ARRAY 100
#define TRUE 1
#define FALSE 0
#define MAX_PAT_LEN 256
#define POINTER_SIZE 8
#define SMALL_ARRAY_SIZE 4

#define DEBUG 1

typedef struct expand_node {
  Type_t type;
  void *next_level;
} Expand_Node_t;

typedef struct suffix_node {
  struct suffix_node *next;
  Char_t str[];
} Suffix_Node_t;

typedef struct fun_call_elmt {
  char fun_name[100];
  unsigned times;
} Fun_Call_Elmt_t;

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

void remove_duplicate(Suffix_Node_t *suf_list);

void print_suffix(Suffix_Node_t *cur_suf);

#endif
