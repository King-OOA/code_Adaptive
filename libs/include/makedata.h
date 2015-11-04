#ifndef MAKEDATA_H
#define MAKEDATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "textools.h"

#define MIN_PAT_LEN 1
#define MAX_PAT_LEN 1000
#define DEBUG

typedef struct pat_node {
    char *pat_str; /*指向模式串*/
    struct pat_node *next;
} patnode_t;

typedef struct pat_set {
    char *pats_file;
    patnode_t *pat_list; /*模式串链表*/
    long total_pats; /*模式串总数*/
    int max_pat_len; /*最大串长*/
    int min_pat_len; /*最小串长*/
    long total_pat_len; /*串长之和*/
    double mean_pat_len; /*平均串长*/
    double pat_len_sd; /*标准差*/
    int pat_len_distri[MAX_PAT_LEN]; /*串长分布，记录每一个长度含有模式串的个数*/
} patset_t;


void cre_rand_text(const char *filename, long text_len, int low, int high);
void cre_rand_pats(const char *filename, long pat_num, int min_pat_len, int max_pat_len, int low, int high);
void extract_pats(char const *text_filename, long pat_num, int min_pat_len, int max_pat_len, char const *pat_filename, char const *forbid);

patset_t *cre_pat_set(const char *);
void des_pat_set(patset_t *);
void print_pat_set(const patset_t *, int);
patnode_t *cre_pat_node(const char *); 




#endif
