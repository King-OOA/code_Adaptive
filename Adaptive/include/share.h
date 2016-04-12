#ifndef SHARE_H
#define SHARE_H
//#pragma once

#include <stdbool.h>

/* 基本类型定义 */
typedef unsigned char UC_T;
typedef char Char_T;
typedef UC_T Pat_Len_T;
typedef unsigned Pat_Num_T;
typedef UC_T Type_T;
typedef UC_T Flag_T;
typedef unsigned Hash_Value_T;

/* 结构类型 */
#define TYPE_NUM    10
#define END         0
#define MAP_1       1
#define MAP_4       2
#define MAP_16      3
#define MAP_48      4
#define MAP_256     5
#define SINGLE_STR  6
#define ARRAY       7
#define MAP_65536   8
#define HASH        9
/* 匹配函数类型 */
#define MATCH_FUN_NUM       10
#define MATCH_MAP_1         0
#define MATCH_MAP_4         1
#define MATCH_MAP_16        2
#define MATCH_MAP_48        3
#define MATCH_MAP_256       4
#define MATCH_SINGLE_STR    5
#define MATCH_ORDERED_ARRAY 6
#define MATCH_BINARY_ARRAY  7
#define MATCH_MAP_65536     8
#define MATCH_HASH          9

#define MAX_PAT_LEN 255
#define ALPHABET_SIZE 256
#define MIN_PERCENT 1.0
#define LLP 100
#define POINTER_SIZE sizeof(void*)
#define BITS_PER_BYTE 8

/* 运行版本 */
#define PROFILING 1  /* 为1时,加入各类统计信息,用于分析程序;否则,仅输出匹配时间,用于最终性能测试*/
#define DEBUG 1 /* 出Bug时用,一般不用 */

/* hash_Table 参数 */
#define L_BITS 6
#define R_BITS 2
#define SEED 50u

/* 装载因子,值越小hash表越大 */
#define LOAD_FACTOR 0.3

/* ndp超过此数值将构造哈希表 */
#define NUM_TO_BUILD_HASH 20
/*  数组中的串数量小于此数值时用顺序查找,否则用二分查找 */
#define SMALL_ARRAY_SIZE 4
/* 双字符快的数量超过该值时,将构造65535_map */
#define NUM_TO_BUILD_65536 100


struct Suf_Node {
  struct Suf_Node *next;
  Char_T str[];
};

typedef struct Tree_Node *Tree_Node_T;
typedef struct Suf_Node *Suf_Node_T;
typedef Tree_Node_T (*Match_Fun_T) (void *, Char_T const **, bool *);

struct Tree_Node {
  Match_Fun_T match_fun;
  void *link;
};

Suf_Node_T cut_head(Suf_Node_T suf_node, Pat_Len_T lsp);
bool same_str(Char_T const *s1, Char_T const *s2, Pat_Len_T len);
void push_children(Tree_Node_T child, Pat_Num_T num);
void print_str(Char_T const *s, Pat_Len_T len, Char_T terminator);

void print_suffix(Suf_Node_T cur_suf);

#endif 
