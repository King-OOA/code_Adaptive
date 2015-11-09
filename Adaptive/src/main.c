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

Queue_t *queue;
unsigned type_num[TYPE_NUM];
static const char *type_name[] =
{"",
 "array", "hash",
 "4 map", "16 map", "48 map",
 "256 map", "65536 map"};

typedef Expand_Node_t * (* Match_Fun_t)(void *, Char_t const **, Bool_t *);

static Expand_Node_t *(*match_fun[]) (void *, Char_t const **, Bool_t *) =
{
     NULL,
     (Match_Fun_t) match_array,
     (Match_Fun_t) match_hash,
     (Match_Fun_t) match_map_4,
     (Match_Fun_t) match_map_16,
     (Match_Fun_t) match_map_48,
     (Match_Fun_t) match_map_256
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
     //printf("%d\n", dif_prf_num);
  
     //exit(EXIT_SUCCESS);  
 
     if (lsp == 1)
	  build_map(expand_node, dif_prf_num);
     else {
	  if (dif_prf_num <= 100)
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
 
   if (is_pat_end) {
      is_matched = TRUE;
      pat_len = s - text;
      *pat_buf++ = ' ';
      memcpy(pat_buf, text, pat_len);
      pat_buf += pat_len;
      is_pat_end = FALSE;
    }
  }
  
  *pat_buf = '\0';
  
  return is_matched;
}

int main(int argc, char **argv)
{
     FILE *pats_fp, *text_fp;
     Expand_Node_t *root;
     int i;
     size_t file_size;
     clock_t start, end;
     Char_t *text_buf, *text_p, *text_end;;
     Char_t pat_buf[1000];
     Bool_t show_match_results = FALSE, show_sta_details = FALSE;
     Char_t opt;
     Char_t *text_file_name, *pats_file_name;
     
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
     
     /* 构建root */
     pats_fp = Fopen(pats_file_name, "r");
     fprintf(stderr, "\nMaking root..."); fflush(stdout);
     start = clock();
     root = make_root(pats_fp);
     end = clock();
     fprintf(stderr, "Done!  \n%f\n",
     	     (double) (end - start) / CLOCKS_PER_SEC);
     Fclose(pats_fp);
     
     /* 预处理,构建自适应匹配结构 */
     fprintf(stderr, "\nConstructing..."); fflush(stdout);
     queue = make_queue();
     in_queue(queue, root);
     start = clock();
     while (!queue_is_empty(queue))
     	  choose_adaptor(out_queue(queue));
     end = clock();
     fprintf(stderr, "Done!  \n%f\n",
     	     (double) (end - start) / CLOCKS_PER_SEC);
     free_queue(queue);
     
     /* 打印各种结构的统计信息 */
     for (i = 0; i < TYPE_NUM; i++)
	  if (type_num[i])
	       printf("%s: %u\n", type_name[i], type_num[i]);
     //exit(EXIT_SUCCESS);  

     /* 读文本 */
     fprintf(stderr, "\nLoading text..."); fflush(stdout);
     start = clock();
     text_fp = Fopen(text_file_name, "r");
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
	  if (match_round(root, text_p, pat_buf) && show_match_results){
	       printf("%ld: %s\n", text_p - text_buf + 1, pat_buf);
	  }
     }
     end = clock();
     fprintf(stderr, "Done!  \n%f\n",
	     (double) (end - start) / CLOCKS_PER_SEC);
}


  /* get_num_and_lsp(root, &total_suf_num, &dif_prf_num, &lsp); */
  /* printf("total: %u, dif: %u, lsp: %d\n", total_suf_num, dif_prf_num, lsp); */
    /* Pat_Num_t total_suf_num = 0, dif_prf_num = 0; */
     /* Pat_Len_t lsp = 0; /\* 最短模式串长 *\/ */
     /* int is_pat_end = 0; */
      //get_num_and_lsp(root, &total_suf_num, &dif_prf_num, &lsp);
     //printf("total_suf_num: %u, dif_prf_num: %u, lsp: %u\n", total_suf_num, dif_prf_num, lsp);
     //build_array(root,  dif_prf_num,  lsp);
     //print_array(root->next_level);

/* void print_expand(Expand_Node_t *expand_node) */
/* { */
/*   switch (expand_node->type) { */
/*   case HASH: print_hash(expand_node->next_level);break; */
/*   case _4_MAP: print_4_map(expand_node->next_level);break; */
/*   case _16_MAP: print_16_map(expand_node->next_level);break; */
/*   case _48_MAP: print_48_map(expand_node->next_level);break; */
/*   case _256_MAP: print_256_map(expand_node->next_level);break; */
/*   case ARRAY : print_leaf(expand_node->next_level); break; */
/*   } */
  
/* } */
