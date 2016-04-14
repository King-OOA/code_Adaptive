#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN_PAT_LEN 1
#define MAX_PAT_LEN 100
#define DEBUG


typedef size_t File_len_t;
typedef size_t Pat_Num_t;
typedef char Char_t;
typedef unsigned short Pat_Len_t;

typedef struct pat_node {
    char *pat_str; /*指向模式串*/
    struct pat_node *next;
} Pat_Node_t;

typedef struct pat_set {
    char *pats_file_name;
    Pat_Node_t *pat_list; /*模式串链表*/
    Pat_Num_t total_pats; /*模式串总数*/
    Pat_Len_t max_pat_len; /*最大串长*/
    Pat_Len_t min_pat_len; /*最小串长*/
    File_len_t total_pat_len; /*串长之和*/
    double mean_pat_len; /*平均串长*/
    double pat_len_sd; /*标准差*/
    Pat_Num_t pat_len_distri[MAX_PAT_LEN]; /*串长分布，记录每一个长度含有模式串的个数*/
} Pat_Set_t;


void cre_rand_pats(const char *filename, Pat_Num_t pat_num, Pat_Len_t min_pat_len, Pat_Len_t max_pat_len, Char_t low, Char_t high);
void cre_rand_text(const char *filename, Pat_Num_t text_len, Char_t low,  Char_t high);
void extract_pats(char const *text_filename, Pat_Num_t pat_num, Pat_Len_t min_pat_len, Pat_Len_t max_pat_len, char const *pat_filename, char const *forbid);

Pat_Set_t *cre_pat_set(const char *);
void des_pat_set(Pat_Set_t *);
void print_pat_set(const Pat_Set_t *, int);
Pat_Node_t *cre_pat_node(const char *); 
