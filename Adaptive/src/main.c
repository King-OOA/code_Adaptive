#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "common.h"
#include "textools.h"
#include "makedata.h"
#include "hash.h"
#include "share.h"
#include "queue.h"
#include "array.h"
#include "map.h"
#include "sorter.h"

Queue_t *queue;
unsigned type_num[TYPE_NUM];
extern unsigned ch_num[];
extern unsigned array_len[];

static const char *type_names[] =
{"",
 "single char", "map 4", "map 16", "map 48",
 "map 256", "single string", "array", "", "hash" };

Fun_Call_Elmt_t fun_calls[] = {
  {"", 0},  {"single char match", 0}, {"map 4 match", 0},
  {"map 16 match", 0}, {"map 48 match", 0}, {"map 256 match", 0},
  {"single string match", 0}, {"array ordered match", 0},
  {"array binary match", 0}, {"hash match", 0}};

typedef Expand_Node_t * (* Match_Fun_t)(void *, Char_t const **, Bool_t *);

static Expand_Node_t *(*match_fun[]) (void *, Char_t const **, Bool_t *) =
{
     NULL,
     (Match_Fun_t) match_single_ch,
     (Match_Fun_t) match_map_4,
     (Match_Fun_t) match_map_16,
     (Match_Fun_t) match_map_48,
     (Match_Fun_t) match_map_256,
     (Match_Fun_t) match_single_str,
     (Match_Fun_t) match_array,
     NULL, /* 二分查找合并在数组匹配中 */
     (Match_Fun_t) match_hash
};

static Suffix_Node_t *make_suffix_node(Char_t const *pat)
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
     Pat_Len_t pat_len;
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
     Suffix_Node_t *pat_list;
     Expand_Node_t *root = CALLOC(1, Expand_Node_t);
     
     pat_list = list_radix_sort(read_pats(pats_fp)); /* 先排序 */
     remove_duplicate(pat_list); /* 再去重 */
    
     root->next_level = pat_list;
     
     return root;
}

static void choose_adaptor(Expand_Node_t *expand_node)
{
  Pat_Num_t total_suf_num, dif_prf_num;
  Pat_Len_t lsp; /* 最短模式串长 */
     
  get_num_and_lsp(expand_node, &total_suf_num, &dif_prf_num, &lsp);
  
  if (lsp == 1) {
    if (dif_prf_num == 1)
      build_single_ch(expand_node); /* 单个字符 */
    else
      build_map(expand_node, dif_prf_num);
  } else {
    if (dif_prf_num == 1)
      build_single_str(expand_node, lsp); /* 单个字符串 */
    else if (dif_prf_num < NUM_TO_BUILD_ARRAY)
      build_array(expand_node, dif_prf_num, lsp);
    else
      build_hash(expand_node, dif_prf_num, lsp);
  }
}

static Bool_t match_round(Expand_Node_t *expand_node, Char_t *text, char *pat_buf)
{
  Bool_t is_matched = FALSE, is_pat_end = FALSE;
  Char_t *s = text;
  Pat_Len_t pat_len;
  
  while (expand_node && expand_node->type != END) {
    expand_node = match_fun[expand_node->type](expand_node->next_level, &s, &is_pat_end);

#if DEBUG
   if (is_pat_end) {
      is_matched = TRUE;
      pat_len = s - text;
      *pat_buf++ = ' ';
      memcpy(pat_buf, text, pat_len);
      pat_buf += pat_len;
      is_pat_end = FALSE;
    }
#endif 
  }
  
  *pat_buf = '\0';
  
  return is_matched;
}

static int fun_cmp(const void *elmt_1, const void *elmt_2)
{
  unsigned times_1 = ((Fun_Call_Elmt_t *) elmt_1)->times;
  unsigned times_2 = ((Fun_Call_Elmt_t *) elmt_2)->times;

  if(times_1 > times_2)
    return 1;
  else if(times_1 == times_2)
    return 0;
  else
    return -1;
}

void print_statistics(Bool_t show_details)
{
  int i;
  unsigned total_fun_calls = 0;
  
  /* 打印各种结构的统计信息 */
  printf("\nNumber of structures:\n\n");
  for (i = 0; i < TYPE_NUM; i++)
    if (type_num[i])
      printf("%s: %u\n", type_names[i], type_num[i]);

  if (show_details) {
    printf("\n\nDiferent sigle char num:\n");
    for (i = 0; i < 256; i++)
      if (ch_num[i])
	printf("%d: %u\n", i, ch_num[i]);
  
    printf("\nArray len distribution:\n");
    for (i = 1; i < NUM_TO_BUILD_ARRAY; i++)
      if (array_len[i])
	printf("%d: %u\n", i, array_len[i]);

    printf("\nNumber of function calls:\n\n");
    for (i = 1; i < TYPE_NUM; i++)
      total_fun_calls += fun_calls[i].times;
    
    qsort(fun_calls + 1, TYPE_NUM - 1, sizeof(Fun_Call_Elmt_t), fun_cmp);
    /* 从大到小输出 */
    for (i = TYPE_NUM - 1; i > 0; i--)
      printf("%s: %u (%.2f%%)\n", fun_calls[i].fun_name, fun_calls[i].times, ((double) fun_calls[i].times / total_fun_calls) * 100.0);
  }

}

int main(int argc, char **argv)
{
  FILE *pats_fp, *text_fp;
  Expand_Node_t *root;
  size_t file_size;
  clock_t start, end;
  Char_t *text_buf, *text_p, *text_end;;
  Char_t pat_buf[1000];
  Bool_t show_match_results = FALSE, show_sta_details = FALSE;
  Char_t opt;
  Char_t *text_file_name, *pats_file_name;
  Bool_t is_matched = FALSE;
  
  /* 处理命令行参数 */
  pats_file_name = argv[1];	/* 模式集文件*/
  text_file_name = argv[2];	/* 文本文件 */
  if (argc > 3) 
    for (argv += 3; *argv && **argv == '-'; argv++) 
      while (opt = *++*argv)
	switch (opt) {
	case 'o' : show_match_results = TRUE; break;
	case 's' : show_sta_details = TRUE; break;
	default : fprintf(stderr, "非法命令行参数!\n"); exit(EXIT_FAILURE);
	}
     

  /* 预处理,构建自适应匹配结构 */
  pats_fp = Fopen(pats_file_name, "r");
  fprintf(stderr, "\nConstructing adaptive matching structure..."); fflush(stdout);
  start = clock();
  root = make_root(pats_fp);	/* 构建根节点 */
  Fclose(pats_fp);
     
  queue = make_queue();
  in_queue(queue, root);
  start = clock();
  while (!queue_is_empty(queue))
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
  putchar('\n');
     
  for (text_p = text_buf; text_p < text_end; text_p++) {
    is_matched = match_round(root, text_p, pat_buf);
#if DEBUG
    if (is_matched && show_match_results) 
      printf("%ld: %s\n", text_p - text_buf + 1, pat_buf);
#endif     
  }
  end = clock();
  fprintf(stderr, "Done!  \n%f\n",
	  (double) (end - start) / CLOCKS_PER_SEC);

  print_statistics(show_sta_details); /* 打印程序运行过程中的各种统计信息 */
}
