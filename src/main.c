#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "adt.h"
#include "bits.h"
#include "patset.h"
#include "mem.h"

#include "hash.h"
#include "share.h"
#include "filter.h"

#include "array.h"
#include "map.h"
#include "sorter.h"
#include "statistics.h"

extern unsigned match_num; /* 任何情况下都要输出,以验证程序正确性 */

#if PROFILING
extern Num_Num_T access_depth[LLP];
extern unsigned total_nodes;
#endif 

#if DEBUG
void print_pat_list(Suf_Node_T list_head)
{
    puts("the pat list");
  
    while (list_head) {
	assert(strlen(list_head->str));
	printf("%s\n", list_head->str);
	list_head = list_head->next;
    }
}
#endif 

/* 有序链表去重,头节点一定会保留 */
static void remove_duplicates(Suf_Node_T pat_list)
{
    Suf_Node_T prev = pat_list, cur = prev->next;
    uint32_t n = 0;
    int8_t result;

    printf("\nRemoving duplicates...\n");

    while (cur)
	if ((result = strcmp(prev->str, cur->str)) == 0) { /* 模式串不等长,均以'\0'结尾*/
	    cur = cur->next; free(prev->next); prev->next = cur; n++;
	} else {
	    prev = cur; cur = cur->next;
	}

    printf("%u patterns removed!\n", n);
}

Pat_Len_T get_lss(Suf_Node_T suf_list)
{
    Pat_Len_T suf_len, lss = MAX_PAT_LEN;

    for (Suf_Node_T cur = suf_list; cur; cur = cur->next)
	if ((suf_len = strlen(cur->str)) < lss)
	    lss = suf_len;

    return lss;
}

static Pat_Num_T get_ndp(Suf_Node_T suf_list, Pat_Len_T lss)
{
    Pat_Num_T ndp = 1;
  
    for (Suf_Node_T prev = suf_list, cur = prev->next; cur; cur = cur->next)
	if (same_str(prev->str, cur->str, lss))
	    continue;
	else {
	    ndp++; prev = cur;
	}

    return ndp;
}

static void build_tree_node(Tree_Node_T t)
{
    /* length of the shortest suffix */
    Pat_Len_T lss = get_lss(t->link);
    /* number of the distinct prefixes */
    Pat_Num_T ndp = get_ndp(t->link, lss); 

#if PROFILING
    total_nodes++;
#endif
 

    /* 自适应策略 */
    if (lss == 1) /* 单字符*/
	build_map(t, ndp);
    else if (ndp < NUM_TO_BUILD_HASH)
	build_array(t, ndp, lss);
    else
	build_hash_table(t, ndp, lss);
}

/* 预处理*/
Stack_T stk;

Filter_T build_AMT(Suf_Node_T pat_list)
{
    fprintf(stderr, "\nBuilding AMT...\n"); fflush(stdout);
    clock_t start = clock();
    
    stk = stack_new();
    Filter_T root = build_filter(pat_list); /* 根节点为过滤器 */

    while (!stack_empty(stk)) /* 构建整个AMT */
	build_tree_node(stack_pop(stk));

    stack_free(&stk);

    clock_t end = clock();
    fprintf(stderr, "Done! (%f)\n",
	    (double) (end - start) / CLOCKS_PER_SEC);

    return root;
}

static bool check_entrance(Tree_Node_T t, Char_T const *entrance, Char_T *matched_pat_buf, bool output)
{
    bool pat_end = false, find_pat = false;
    Char_T const *p = entrance;

#if PROFILING
    uint8_t depth = 0;
#endif
    while (t && t->link) { /*非: 匹配失败或已到达叶节点 */
	t = t->match_fun(t->link, &p, &pat_end);
#if PROFILING
	depth++;
#endif
	if (pat_end) {
	    match_num++;
	    find_pat = true;
	    pat_end = false;
#if PROFILING
	    if (output) {
		Pat_Len_T pat_len = p - entrance;
		*matched_pat_buf++ = ' ';  /* 模式串以空格分隔 */
		memcpy(matched_pat_buf, entrance, pat_len); /* 复制一个完整模式串 */
		matched_pat_buf += pat_len;
	    }
#endif
	}
    }
  
    *matched_pat_buf = '\0';

#if PROFILING
    access_depth[depth].num_1 = depth;
    access_depth[depth].num_2++;
#endif

    return find_pat;
}
 
/* 匹配文本*/
void matching(Filter_T filter, Char_T *text_buf, size_t text_len, bool output)
{
    fprintf(stderr, "\nMatching...\n"); fflush(stdout);
    Char_T matched_pat_buf[500];
  
    Pat_Len_T block_size = filter->block_size;
    Pat_Len_T window_size = filter->window_size;
    Pat_Len_T last_pos = window_size - block_size;
    static Bitmap_T mb = -1; /* 全"1" */
    Bitmap_T *bitmap = filter->bitmap;
    uint64_t hit_num = 0;

    clock_t start = clock();
    Char_T *entrance = text_buf;
    while (entrance <= text_buf + text_len - window_size) {
	uint32_t v = block_123(entrance + last_pos, block_size);
	mb &= bitmap[v];

	if (test_bit(&mb, last_pos)) {
	    hit_num++;
	    bool find_pat = check_entrance(filter->children + v, entrance, matched_pat_buf, output);
#if PROFILING
	    if (find_pat && output)
		printf("%ld: %s\n", entrance - text_buf + 1, matched_pat_buf);
#endif
	}
      
	Pat_Len_T skip = 0; /* 至少跳一步 */
	do {
	    mb >>= 1;
	    skip++;
	    set_bit(&mb, 0); /* 首位置1 */
	} while (!test_bit(&mb, last_pos)); /* 末位不是1 */

	entrance += skip;
    }
    clock_t end = clock();

    fprintf(stderr, "\nDone! (%f) total skip: %lu (%.2f%%)\n",
	    (double) (end - start) / CLOCKS_PER_SEC, (text_len - hit_num), (double) (text_len - hit_num) / text_len);
}

/* 将模式集中的模式串,拷贝到后缀链表节点中 */
static void copy_pat(void **pat_p, void *next_pp)
{
#define SUF **((Suf_Node_T **) next_pp) 

    VNEW0(SUF, strlen(*pat_p)+1, Char_T);
    strcpy((SUF)->str, *pat_p);
    *((Suf_Node_T **) next_pp) = &(SUF)->next;
}

int main(int argc, char **argv)
{
    bool output = false, show_sta_info = false;
    Char_T opt;
 
    /* 处理命令行参数 */
    if (argc > 3) /* 解析随后的参数 */
	for (char **arg = argv + 3; *arg && **arg == '-'; arg++)
	    while ((opt = *++*arg))
		switch (opt) {
		case 'o' : output = true; break; /* -o表示显示匹配结果 */
		case 's' : show_sta_info = true; break;   /* -s表示显示统计信息 */
		default : fprintf(stderr, "非法命令行参数!\n"); exit(EXIT_FAILURE);
		}

    /* 构建模式集 */
    Patset_T patset = patset_new(argv[1]);
    Suf_Node_T pat_list, *next_p = &pat_list;
    /* 将模式集中的模式串依次拷贝到pat_list中 */
    list_traverse(patset->pat_list, copy_pat, &next_p);
    /* 读取模式集,并按模式串字典序排序 */
    pat_list = list_radix_sort(pat_list); 
    remove_duplicates(pat_list); /* 去掉模式集中重复的元素 */    
    
    /* 构建AMT,根节点为过滤器 */
    Filter_T root = build_AMT(pat_list);

    /* 读文本 */
    fprintf(stderr, "\nLoading text...\n"); fflush(stdout);
    size_t file_size;
    Char_T *text_buf = load_file(argv[2], &file_size); /* argv[2]是文本文件名 */
    fprintf(stderr, "Done!\n");
    
    /* 匹配文本 */
    matching(root, text_buf, file_size, output);

    fprintf(stderr, "\nTotal matched number: %u\n", match_num);
  
#if PROFILING
    if (show_sta_info)
	print_statistics(file_size); /* 打印程序运行过程中的各种统计信息 */
#endif

}
