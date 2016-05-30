#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "statistics.h"
#include "share.h"
#include <assert.h>

uint32_t match_num;

#if PROFILING

uint32_t total_nodes;

Str_Num_T type_num[] =
  {{"", 0}, {"Map 1", 0}, {"Map 4", 0}, {"Map 16", 0}, {"Map 48", 0},
   {"Map 256", 0}, {"Single String", 0}, {"Array", 0},{"Map 65536", 0}, {"Hash", 0}, {"Merged String", 0}};

Str_Num_T fun_calls[] = {
  {"Map 1 match", 0}, {"Map 4 match", 0},
  {"Map 16 match", 0}, {"Map 48 match", 0}, {"Map 256 match", 0},
  {"Single string match", 0}, {"Array ordered match", 0},
  {"Array binary match", 0}, {"Map 65536 match", 0}, {"Hash match", 0}, {"Merged string match", 0}};

Num_Num_T map_size[ALPHABET_SIZE];
Num_Num_T array_size[NUM_TO_BUILD_HASH+1]; /* 0元素不用 */
Num_Num_T array_len[MAX_PAT_LEN+1];
Num_Num_T access_depth[LLP];
 
static int str_num_cmp(const void *elmt_1, const void *elmt_2)
{
  uint32_t times_1 = ((Str_Num_T *) elmt_1)->num;
  uint32_t times_2 = ((Str_Num_T *) elmt_2)->num;

  if(times_1 > times_2)
    return 1;
  else if(times_1 == times_2)
    return 0;
  else
    return -1;
}

static int num_num_cmp(const void *elmt_1, const void *elmt_2)
{
  uint32_t value_1 = ((Num_Num_T *) elmt_1)->num_2;
  uint32_t value_2 = ((Num_Num_T *) elmt_2)->num_2;

  if(value_1 > value_2)
    return 1;
  else if(value_1 == value_2)
    return 0;
  else
    return -1;
}

/* 计算num中的位数 */
static int8_t get_digits(uint32_t num)
{
  int8_t i;

  for (i = 1; num / 10; num /= 10, i++)
    ;
  
  return i;
}

void print_statistics(size_t text_len)
{
  int8_t digits;
  uint32_t total_num, num, total_calls;
  double percent;
  
  /* 打印各种结构的数量 */
  printf("\nAMT: %u nodes\n", total_nodes);
  total_num = 0;
  for (int8_t i = 1; i < TYPE_NUM; i++)
    total_num += type_num[i].num;
  qsort(type_num + 1,  TYPE_NUM - 1, sizeof(Str_Num_T), str_num_cmp);
  digits = get_digits(type_num[TYPE_NUM-1].num); /* 最大数的位数 */
  /* 从大到小输出 */
  for (int8_t i = TYPE_NUM - 1; i > 0; i--)
    printf("%-13s : %*u (%5.2f%%)\n", type_num[i].name, digits, type_num[i].num, ((double) type_num[i].num / total_num) * 100.0);

  /* 打印访问深度 */
  total_calls = total_num = 0;
  for (int32_t i = 1; i < LLP; i++) {
    total_num += access_depth[i].num_2;
    total_calls += i * access_depth[i].num_2;
  }
  printf("\nAccess depth: %lf (avg)\n", (double) total_calls / text_len);
  qsort(access_depth, LLP, sizeof(Num_Num_T), num_num_cmp);
  digits = get_digits(access_depth[LLP-1].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (int32_t i = LLP - 1; (num = access_depth[i].num_2); i--)
    if ((percent = ((double) num / total_num) * 100.0) > 0)
      printf("%2u: %*u (%5.2f%%)\n", access_depth[i].num_1, digits, num, percent);

  /* 打印匹配函数调用次数 */
  printf("\nFunction calls:\n");
  total_num = 0;
  for (int8_t i = 0; i < MATCH_FUN_NUM; i++)
    total_num += fun_calls[i].num;
  assert(total_num == total_calls);
  qsort(fun_calls, MATCH_FUN_NUM, sizeof(Str_Num_T), str_num_cmp);
  digits = get_digits(fun_calls[MATCH_FUN_NUM-1].num); /* 最大数的位数 */
  /* 从大到小输出 */
  for (int8_t i = TYPE_NUM - 1; i >= 0; i--)
    printf("%-20s : %*u (%5.2f%%)\n", fun_calls[i].name, digits, fun_calls[i].num, ((double) fun_calls[i].num / total_num) * 100.0);

  /* 打印不同长度map的数量 */
  printf("\nNumber of map with different size:\n");
  total_num = 0;
  for (int16_t i = 1; i < ALPHABET_SIZE; i++)
    total_num += map_size[i].num_2;
  qsort(map_size, ALPHABET_SIZE, sizeof(Num_Num_T), num_num_cmp);
  digits = get_digits(map_size[ALPHABET_SIZE-1].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (int16_t i = ALPHABET_SIZE - 1; (num = map_size[i].num_2); i--)
    if ((percent = ((double) num / total_num) * 100.0) > MIN_PERCENT)
      printf("%2u: %*u (%5.2f%%)\n", map_size[i].num_1, digits, num, percent);
  
  /* 打印不同长度数组的数量 */
  printf("\nNumber of array with different size:\n");
  total_num = 0;
    for (int32_t i = 1; i < NUM_TO_BUILD_HASH + 1; i++)
    total_num += array_size[i].num_2;
  qsort(array_size, NUM_TO_BUILD_HASH + 1, sizeof(Num_Num_T), num_num_cmp);
  digits = get_digits(array_size[NUM_TO_BUILD_HASH].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (int32_t i = NUM_TO_BUILD_HASH; (num = array_size[i].num_2); i--)
    if ((percent = ((double) num / total_num) * 100.0) > MIN_PERCENT)
      printf("%2u: %*u (%5.2f%%)\n", array_size[i].num_1, digits, num, percent);

    /* 打印不同长度数组的数量 */
  printf("\nNumber of array with different string length:\n");
  total_num = 0;
  for (int32_t i = 1; i < MAX_PAT_LEN + 1; i++)
    total_num += array_len[i].num_2;
  qsort(array_len, MAX_PAT_LEN + 1, sizeof(Num_Num_T), num_num_cmp);
  digits = get_digits(array_len[MAX_PAT_LEN].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (int32_t i = MAX_PAT_LEN; (num = array_len[i].num_2); i--)
    if ((percent = ((double) num / total_num) * 100.0) > MIN_PERCENT)
      printf("%2u: %*u (%5.2f%%)\n", array_len[i].num_1, digits, num, percent);
}

#endif
