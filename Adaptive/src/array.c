#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share.h"
#include "common.h"
#include "queue.h"
#include "array.h"

extern Queue_t *queue;
extern Pat_Num_t type_num[];

static Str_Array_t *make_array(Pat_Num_t str_num, Pat_Len_t str_len)
{
     Str_Array_t *new_array = VMALLOC(Str_Array_t, Str_Elmt_t, str_num);
     
     new_array->str_num = str_num;
     new_array->str_len = str_len;
     memset(new_array->array, 0, str_num * sizeof(Str_Elmt_t));
     
     return new_array;
}

void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len)
{
  Suffix_Node_t *cur_suf, *next_suf, *suf_list, **next_p;
  Str_Array_t *str_array = make_array(str_num, str_len); /* 构建str_array */
  Str_Elmt_t *str_elmt = str_array->array;
  Pat_Num_t n = str_num;

  if (str_len > POINTER_SIZE) { /* 节点外 */
    while (n--) (str_elmt++)->str.p = MALLOC(str_len, Char_t); /* 为每个节点,在节点外分配空间 */
    
    /* 把第一个字符串拷入数组的第一个元素 */
    str_elmt = str_array->array;
    memcpy(str_elmt->str.p, ((Suffix_Node_t *) expand_node->next_level)->str, str_len);
    next_p = (Suffix_Node_t **) &str_elmt->expand_node.next_level;
    
    for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
      next_suf = cur_suf->next;
      if (!same_str(str_elmt->str.p, cur_suf->str, str_len)) { /* 如果遇到新的不相等的字符串 */
	memcpy((++str_elmt)->str.p, cur_suf->str, str_len);
	*next_p = NULL;	next_p = (Suffix_Node_t **) &str_elmt->expand_node.next_level; /*原链表终止, 指向新链表头 */
      }

      if (cur_suf = cut_head(cur_suf, str_len)) { 
	*next_p = cur_suf; next_p = &cur_suf->next;
      } else
	str_elmt->pat_end_flag = 1;
    }
  } else {			/* 节点内 */
    memcpy(str_elmt->str.buf, ((Suffix_Node_t *) expand_node->next_level)->str, str_len);
    next_p = (Suffix_Node_t **) &str_elmt->expand_node.next_level;

    for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) {
      next_suf = cur_suf->next; 
      if (!same_str(str_elmt->str.buf, cur_suf->str, str_len)) {
	memcpy((++str_elmt)->str.buf, cur_suf->str, str_len);
	*next_p = NULL; next_p = (Suffix_Node_t **) &str_elmt->expand_node.next_level;
      }

      if (cur_suf = cut_head(cur_suf, str_len)) {
	*next_p = cur_suf; next_p = &cur_suf->next;
      } else
	str_elmt->pat_end_flag = 1;
    }
  }

  expand_node->next_level = str_array;
  expand_node->type = ARRAY;
  type_num[ARRAY]++;
     
  /* 加入到队列 */
  for (str_elmt = str_array->array; str_num; str_elmt++, str_num--)
    if (suf_list = str_elmt->expand_node.next_level) {
      in_queue(queue, &str_elmt->expand_node);
    }
}

/*有序查找*/
inline static Expand_Node_t *ordered_match(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end)
{
     Pat_Len_t str_len = str_array->str_len;
     Pat_Num_t str_num;
     Char_t const *s = *text;
     Str_Elmt_t *str_elmt;
     int result;

     if (str_len > POINTER_SIZE)  /* 节点外 */
	  for (str_num = str_array->str_num, str_elmt = str_array->array;
	       str_num && (result = str_n_cmp(str_elmt->str.p, s, str_len)) < 0;
	       str_num--, str_elmt++)
	       ;
     else 			/* 节点内 */
	  for (str_num = str_array->str_num, str_elmt = str_array->array;
	       str_num && (result = str_n_cmp(str_elmt->str.buf, s, str_len)) < 0;
	       str_num--, str_elmt++)
	       ;

     if (str_num == 0 || result > 0) 	/* 没找到 */
	  return NULL;

     *is_pat_end = str_elmt->pat_end_flag;
     *text += str_len;
     
     return &str_elmt->expand_node;
}

/* 二分查找 */
inline static Expand_Node_t *binary_match(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end)
{
  int low = 0, high = str_array->str_num - 1, mid;
  Str_Elmt_t *array = str_array->array;
  Char_t const *s = *text;
  Pat_Len_t str_len = str_array->str_len;
  int result;
  
  if (str_len > POINTER_SIZE) 
    while (low <= high) {
      mid = (low + high) >> 1;

      if ((result = str_n_cmp(s, array[mid].str.p, str_len)) == 0) {
	*is_pat_end = array[mid].pat_end_flag;
	*text += str_len;
	return &array[mid].expand_node;
      } else if (result < 0)
	high = mid - 1;
      else
	low = mid + 1;
    }
  else  			/* 节点内 */
    while (low <= high) {
      mid = (low + high) >> 1;

      if ((result = str_n_cmp(s, array[mid].str.buf, str_len)) == 0) {
	*is_pat_end = array[mid].pat_end_flag;
	*text += str_len;
	return &array[mid].expand_node;
      } else if (result < 0)
	high = mid - 1;
      else
	low = mid + 1;
    }

  return NULL;
}

Expand_Node_t *match_array(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end)
{
  if (str_array->str_num <= SMALL_ARRAY_SIZE)
    return ordered_match(str_array, text, is_pat_end);
  else
    return binary_match(str_array, text, is_pat_end);
    
}

void print_array(Str_Array_t *str_array)
{
  Str_Elmt_t *str_elmt;
  Pat_Len_t str_len = str_array->str_len;
  Pat_Num_t n;

  if (str_len > POINTER_SIZE)
    for (str_elmt = str_array->array, n = str_array->str_num; n; str_elmt++, n--) {
      print_str(str_elmt->str.p, str_len,  ':');
      if (str_elmt->pat_end_flag)
	putchar('*');
      putchar('\n');
      print_suffix(str_elmt->expand_node.next_level);
    }
  else
    for (str_elmt = str_array->array, n = str_array->str_num; n; str_elmt++, n--) {
      print_str(str_elmt->str.buf, str_len,  ':');
      if (str_elmt->pat_end_flag)
	putchar('*');
      putchar('\n');
      print_suffix(str_elmt->expand_node.next_level);
    }
}

/* void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len) */
/* { */
/*      Suffix_Node_t *cur_suf, *next_suf; */
/*      Str_Array_t *str_array = make_array(str_num, str_len); /\* 构建str_array *\/ */
/*      int n; */
/*      Str_Elmt_t *str_elmt; */
     
/*      if (str_len > POINTER_SIZE)  /\* 节点外 *\/ */
/* 	  for (str_num = 0, cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /\* str_num表示当前在数组中已存放的串数 *\/ */
/* 	       next_suf = cur_suf->next; */
	       
/* 	       for (n = str_num, str_elmt = str_array->array; n && !same_str(str_elmt->str.p, cur_suf->str, str_len); str_elmt++, n--) */
/* 		    ; */

/* 	       if (n == 0) {/\* 没找到 *\/ */
/* 		    str_elmt->str.p = MALLOC(str_len, Char_t); */
/* 		    memcpy(str_elmt->str.p, cur_suf->str, str_len); */
/* 		    str_num++; */
/* 	       } */
	       
/* 	       if (cur_suf = cut_head(cur_suf, str_len)) */
/* 		    insert_to_expand(&str_elmt->expand_node, cur_suf); */
/* 	       else */
/* 		    str_elmt->pat_end_flag = 1; */
/* 	  } */
/*      else  /\* 节点内 *\/ */
/* 	  for (str_num = 0, cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { /\* str_num表示当前在数组中已有的串数 *\/ */
/* 	       next_suf = cur_suf->next; */
	       
/* 	       for (n = str_num, str_elmt = str_array->array; n && !same_str(str_elmt->str.buf, cur_suf->str, str_len); str_elmt++, n--) */
/* 		    ; */

/* 	       if (n == 0) {/\* 没找到 *\/ */
/* 		    memcpy(str_elmt->str.buf, cur_suf->str, str_len); */
/* 		    str_num++; */
/* 	       } */
	       
/* 	       if (cur_suf = cut_head(cur_suf, str_len)) */
/* 		    insert_to_expand(&str_elmt->expand_node, cur_suf); */
/* 	       else */
/* 		    str_elmt->pat_end_flag = 1; */
/* 	  } */

/*      expand_node->next_level = str_array; */
/*      expand_node->type = ARRAY; */
/*      type_num[ARRAY]++; */
     
/*      /\* 加入到队列 *\/ */
/*      for (str_elmt = str_array->array; str_num; str_elmt++, str_num--) */
/* 	  if (str_elmt->expand_node.next_level) */
/* 	       in_queue(queue, &str_elmt->expand_node); */
/* } */

/*无序查找  */

/* Expand_Node_t *match_array(Str_Array_t *str_array, Char_t const **text, Bool_t *is_pat_end) */
/* { */
/*      Pat_Len_t str_len = str_array->str_len; */
/*      Pat_Num_t str_num; */
/*      Char_t const *s = *text; */
/*      Str_Elmt_t *str_elmt; */

/*      if (str_len > POINTER_SIZE)  /\* 节点外 *\/ */
/* 	  for (str_num = str_array->str_num, str_elmt = str_array->array; */
/* 	       str_num && !same_str(s, str_elmt->str.p, str_len); */
/* 	       str_num--, str_elmt++) */
/* 	       ; */
/*      else 			/\* 节点内 *\/ */
/* 	  for (str_num = str_array->str_num, str_elmt = str_array->array; */
/* 	       str_num && !same_str(s, str_elmt->str.buf, str_len); */
/* 	       str_num--, str_elmt++) */
/* 	       ; */

/*      if (str_num == 0) 	/\* 没找到 *\/ */
/* 	  return NULL; */

/*      *is_pat_end = str_elmt->pat_end_flag; */
/*      *text += str_len; */
     
/*      return &str_elmt->expand_node; */
/* } */


/* void build_array(Expand_Node_t *expand_node, Pat_Num_t str_num, Pat_Len_t str_len) */
/* { */
/*   Suffix_Node_t *cur_suf, *next_suf, *suf_list, **next_p; */
/*   Str_Array_t *str_array = make_array(str_num, str_len); /\* 构建str_array *\/ */
/*   Str_Elmt_t *str_elmt = str_array->array; */
  
/*   if (str_len > POINTER_SIZE) { /\* 节点外 *\/ */
/*     str_elmt->str.p = MALLOC(str_len, Char_t); /\* 把第一个字符串拷入数组的第一个元素 *\/ */
/*     memcpy(str_elmt->str.p, ((Suffix_Node_t *) expand_node->next_level)->str, str_len); */

/*     for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*       next_suf = cur_suf->next; cur_suf->next = NULL; */
/*       if (!same_str(str_elmt->str.p, cur_suf->str, str_len)) { /\* 如果遇到新的不相等的字符串 *\/ */
/* 	(++str_elmt)->str.p = MALLOC(str_len, Char_t);	       /\* 则先将其拷入新的数组元素中 *\/ */
/* 	memcpy(str_elmt->str.p, cur_suf->str, str_len); */
/*       } */

/*       if (cur_suf = cut_head(cur_suf, str_len)) {  */
/* 	cur_suf->next = str_elmt->expand_node.next_level; */
/* 	str_elmt->expand_node.next_level = cur_suf; */
/*       } */
/*       else */
/* 	str_elmt->pat_end_flag = 1; */
/*     } */
/*   } else {			/\* 节点内 *\/ */
/*     memcpy(str_elmt->str.buf, ((Suffix_Node_t *) expand_node->next_level)->str, str_len); */

/*     for (cur_suf = expand_node->next_level; cur_suf; cur_suf = next_suf) { */
/*       next_suf = cur_suf->next; cur_suf->next = NULL; */
/*       if (!same_str(str_elmt->str.buf, cur_suf->str, str_len)) { */
/* 	memcpy((++str_elmt)->str.buf, cur_suf->str, str_len); */
/*       } */

/*       if (cur_suf = cut_head(cur_suf, str_len)) { */
/* 	cur_suf->next = str_elmt->expand_node.next_level; */
/* 	str_elmt->expand_node.next_level = cur_suf; */
/*       } */
/*       else */
/* 	str_elmt->pat_end_flag = 1; */
/*     } */
/*   } */

/*   expand_node->next_level = str_array; */
/*   expand_node->type = ARRAY; */
/*   type_num[ARRAY]++; */
     
/*   /\* 加入到队列 *\/ */
/*   for (str_elmt = str_array->array; str_num; str_elmt++, str_num--) */
/*     if (suf_list = str_elmt->expand_node.next_level) { */
/*       str_elmt->expand_node.next_level = list_radix_sort(suf_list); */
/*       //     remove_duplicate(str_elmt->expand_node.next_level); */
/*       in_queue(queue, &str_elmt->expand_node); */
/*     } */
/* } */
