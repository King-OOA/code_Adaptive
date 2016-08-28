#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "patset.h"

/* 基本类型定义 */
typedef unsigned char UC_T;
typedef char Char_T;
typedef UC_T Type_T;
typedef UC_T Flag_T;
typedef uint32_t Hash_Value_T;
typedef uint32_t Bitmap_T;

/* 结构类型 */
#define TYPE_NUM    11
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
#define MERGED_STR  10

/* 匹配函数类型 */
#define MATCH_FUN_NUM       11
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
#define MATCH_MERGED_STR    10

#define MAX_PAT_LEN 100
#define ALPHABET_SIZE 256
#define MIN_PERCENT 1.0
#define LLP 100 /* length of the longest pattern */
#define POINTER_SIZE sizeof(void*)
#define BITS_PER_BYTE 8

/* 运行版本 */
#define PROFILING 1  /* 为1时,加入各类统计信息,用于分析程序;否则,仅输出匹配时间,用于最终性能测试*/
#define DEBUG 0 /* 出Bug时用,一般不用 */

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

struct Output_Buf {
     Char_T buf[5000];	/* 存放匹配成功的模式串 */
     Char_T *cur_pos;	/* buf中当前位置 */
} *output_buf;


typedef struct Tree_Node *Tree_Node_T;
typedef struct Suf_Node *Suf_Node_T;
typedef Tree_Node_T (*Match_Fun_T) (Tree_Node_T, Char_T const *, Char_T const **);

bool output; /* 是否输出结果 */


struct Tree_Node {
     Match_Fun_T match_fun;
     void *link;
};

Suf_Node_T cut_head(Suf_Node_T suf_node, Pat_Len_T lsp);
extern inline bool same_str(Char_T const *s1, Char_T const *s2, Pat_Len_T len);
extern int8_t str_cmp(UC_T const *s1, UC_T const *s2, Pat_Len_T len);
void push_children(Tree_Node_T child, Pat_Num_T num);
void print_str(Char_T const *s, Pat_Len_T len, Char_T terminator);
uint32_t block_123(UC_T const *p, int8_t k);
void copy_to_output(Char_T const *begin, Char_T const *end);



void print_suffix(Suf_Node_T cur_suf);
