#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "textools.h"
//#include "makedata.h"
#include "hash.h"
#include "share.h"
#include "queue.h"
#include "array.h"
#include "map.h"
#include "sorter.h"
#include "statistics.h"
#include "binary.h"

Queue_t *queue;

static Suffix_Node_t *make_suffix_node(Char_t const *pat, Pat_Len_t pat_len)
{
     Suffix_Node_t *new_suf_node;

     new_suf_node = VMALLOC(Suffix_Node_t, char, pat_len);
     strcpy(new_suf_node->str, pat);
     new_suf_node->next = NULL;

     return new_suf_node;
}

static Suffix_Node_t *read_pats(FILE *pats_fp)
{
  Char_t buf[MAX_PAT_LEN+1]; /*模式串缓存,包括换行符*/
  Pat_Len_t pat_len;
  Char_t *line_break = NULL; /*换行符指针*/
  Suffix_Node_t *list_head = NULL, *new_suf_node = NULL; /* 链表头指针 */

  while (fgets(buf, sizeof(buf), pats_fp)) {
    if ((line_break = strchr(buf, '\n')))
      *line_break = '\0';
    if ((pat_len = strlen(buf))) { /* 必须是非空模式串 */
      new_suf_node = make_suffix_node(buf, pat_len+1); /* buf末尾包含'\0' */
      new_suf_node->next = list_head; /* 挂到链表头 */
      list_head = new_suf_node; /* 新连表头 */
    }
  }

  return list_head;
}

/* 有序链表去重,头节点一定会保留 */
static void remove_duplicates(Suffix_Node_t *pat_list)
{
  Suffix_Node_t *left = pat_list, *right = left->next;

  while (right)
    if (strcmp(left->str, right->str) == 0) { /* 模式串不等长,均以'\0'结尾*/
      right = right->next; free(left->next); left->next = right;
    } else {
      left = right; right = left->next;
    }
}

static void get_ndp_and_lss(Expand_Node_t const *expand_node, Pat_Num_t *ndp_p, Pat_Len_t *lss_p)
{
  Suffix_Node_t *cur_suf, *left, *right;
  Pat_Len_t suf_len, lss = MAX_PAT_LEN;
  Pat_Num_t ndp = 1;

  /* 第一遍,确定lss */
  for (cur_suf = expand_node->next_level; cur_suf; cur_suf = cur_suf->next)
    if ((suf_len = strlen(cur_suf->str)) < lss)
      lss = suf_len;

  *lss_p = lss;

  /* 第二遍,确定ndp */
  left = expand_node->next_level; right = left->next;
  while (right)
    if (same_str(left->str, right->str, lss))
      right = right->next;
    else {
      left = right; right = right->next; ndp++;
    }

  *ndp_p = ndp;
}

extern unsigned total_nodes;

static void choose_adaptor(Expand_Node_t *expand_node)
{
  Pat_Num_t ndp; /* number of the distinct prefix */
  Pat_Len_t lss; /* length of the shortest suffix */

  get_ndp_and_lss(expand_node, &ndp, &lss); 

#if DEBUG
  total_nodes++;
#endif
  /* 自适应策略 */
  if (lss == 1) { /* 单字符*/
    build_map(expand_node, ndp);
  } else { /* 多字符 */
    if (ndp < NUM_TO_BUILD_HASH)
      build_array(expand_node, ndp, lss);
    else
      build_hash_table(expand_node, ndp, lss);
  }
}

extern unsigned match_num;
extern Num_Num_t access_depth[LLP];

static Bool_t match_round(Expand_Node_t *expand_node, Char_t const *match_entr, Char_t *pat_buf)
{
  Bool_t is_matched = FALSE, is_pat_end = FALSE;
  Char_t const *p = match_entr;
  Pat_Len_t pat_len;
  int depth = 0;

  while (expand_node && expand_node->next_level) { /* 当前匹配成功且不是叶节点时,继续 */
    expand_node = expand_node->match_fun(expand_node->next_level, &p, &is_pat_end);
    
#if DEBUG
    depth++;

   if (is_pat_end) {
     match_num++;
     is_matched = TRUE;
     pat_len = p - match_entr;
     *pat_buf++ = ' ';
     memcpy(pat_buf, match_entr, pat_len);
     pat_buf += pat_len;
     is_pat_end = FALSE;
    }
#endif
  }
  
#if DEBUG
  access_depth[depth].num_1 = depth;
  access_depth[depth].num_2++;
  *pat_buf = '\0';
#endif 

  return is_matched;
}

int main(int argc, char **argv)
{
  FILE *pats_fp, *text_fp;
  Expand_Node_t *root;
  size_t file_size;
  clock_t start, end;
  Char_t *text_buf, *match_entr, *text_end;;
  Char_t pat_buf[1000]; /* 用于存放匹配成功的模式 */
  Bool_t output = FALSE, show_sta_info = FALSE;
  Char_t opt;
  Char_t *text_file_name, *pats_file_name;
  Bool_t is_matched = FALSE;
  Suffix_Node_t *pat_list;
  
  /* char *p=NULL; */
  /* strlen(p); */
  /* exit(EXIT_FAILURE); */
 /* 处理命令行参数 */
  pats_file_name = argv[1];	/* 第一个参数为模式集文件名*/
  text_file_name = argv[2];	/* 第二个参数文本文件名*/
  if (argc > 3) 		/* 解析随后的参数 */
    for (argv += 3; *argv && **argv == '-'; argv++)
      while ((opt = *++*argv))
	switch (opt) {
	case 'o' : output = TRUE; break; /* -o表示显示匹配结果 */
	case 's' : show_sta_info = TRUE; break;   /* -s表示显示统计信息 */
	default : fprintf(stderr, "非法命令行参数!\n"); exit(EXIT_FAILURE);
	}

  /* 预处理*/
  pats_fp = Fopen(pats_file_name, "rb");
  fprintf(stderr, "\nConstructing Adaptive Structures..."); fflush(stdout);
  start = clock();
  pat_list = list_radix_sort(read_pats(pats_fp)); /* 读取模式集,并按模式串字典序排序 */
  remove_duplicates(pat_list); /* 去掉模式集中重复的元素 */
  root = CALLOC(1, Expand_Node_t); /* 构建根节点,本质是一个expand_node */
  root->next_level = pat_list;
  Fclose(pats_fp);

  queue = make_queue();
  in_queue(queue, root);
  while (!queue_is_empty(queue)) /* 构建整个AMT */
    choose_adaptor(out_queue(queue));
  end = clock();
  fprintf(stderr, "Done!  \n%f\n",
	  (double) (end - start) / CLOCKS_PER_SEC);
  free_queue(queue);

  /* 读文本 */
  fprintf(stderr, "\nLoading text..."); fflush(stdout);
  start = clock();
  text_fp = Fopen(text_file_name, "rb");
  file_size = get_file_size(text_fp);
  text_buf = MALLOC(file_size + 1, Char_t);
  fread(text_buf, file_size, 1, text_fp);
  text_buf[file_size] = '\0';
  end = clock();
  fprintf(stderr, "Done!  \n%f\n",
	  (double) (end - start) / CLOCKS_PER_SEC);

  /* 匹配文本*/
  fprintf(stderr, "\nMatching..."); fflush(stdout);
  start = clock();
  text_end = text_buf + file_size - 1;

  for (match_entr = text_buf; match_entr < text_end; match_entr++) {
    is_matched = match_round(root, match_entr, pat_buf);
#if DEBUG
    if (is_matched && output)
      printf("%ld: %s\n", match_entr - text_buf + 1, pat_buf);
#endif
  }
  end = clock();
  fprintf(stderr, "Done!  \n%f\n",
	  (double) (end - start) / CLOCKS_PER_SEC);

  fprintf(stderr, "\nTotal matched number: %u\n", match_num);

#if DEBUG
  if (show_sta_info)
    print_statistics(); /* 打印程序运行过程中的各种统计信息 */
#endif

}
