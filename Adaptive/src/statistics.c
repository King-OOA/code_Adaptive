#include <stdio.h>
#include <stdlib.h>
#include "statistics.h"
#include "share.h"

#if DEBUG

unsigned match_num;

Str_Num_t type_num[] =
  {{"", 0}, {"single char", 0}, {"map 4", 0}, {"map 16", 0}, {"map 48", 0},
   {"map 256", 0}, {"single string", 0}, {"array", 0}, {"hash", 0}};

Str_Num_t fun_calls[] = {
  {"single char match", 0}, {"map 4 match", 0},
  {"map 16 match", 0}, {"map 48 match", 0}, {"map 256 match", 0},
  {"single string match", 0}, {"array ordered match", 0},
  {"array binary match", 0}, {"hash match", 0}};

Num_Num_t map_size[ALPHABET_SIZE];
Num_Num_t array_size[NUM_TO_BUILD_ARRAY+1]; /* 0元素不用 */
 
static int str_num_cmp(const void *elmt_1, const void *elmt_2)
{
  unsigned times_1 = ((Str_Num_t *) elmt_1)->num;
  unsigned times_2 = ((Str_Num_t *) elmt_2)->num;

  if(times_1 > times_2)
    return 1;
  else if(times_1 == times_2)
    return 0;
  else
    return -1;
}

static int num_num_cmp(const void *elmt_1, const void *elmt_2)
{
  unsigned value_1 = ((Num_Num_t *) elmt_1)->num_2;
  unsigned value_2 = ((Num_Num_t *) elmt_2)->num_2;

  if(value_1 > value_2)
    return 1;
  else if(value_1 == value_2)
    return 0;
  else
    return -1;
}

/* 计算num中的位数 */
static int get_digits(unsigned num)
{
  int i;

  for (i = 1; num / 10; num /= 10, i++)
    ;
  
  return i;
}

void print_statistics(void)
{
  int i, digits;
  unsigned total_num, num;
  double percent;
  
  /* 打印各种结构的数量 */
  printf("\nNumber of Different Structures:\n\n");
  for (total_num = 0, i = 1; i < TYPE_NUM; i++)
    total_num += type_num[i].num;
  qsort(type_num + 1,  TYPE_NUM - 1, sizeof(Str_Num_t), str_num_cmp);
  digits = get_digits(type_num[TYPE_NUM-1].num); /* 最大数的位数 */
  /* 从大到小输出 */
  for (i = TYPE_NUM - 1; i > 0; i--)
    printf("%-13s : %*u (%5.2f%%)\n", type_num[i].name, digits, type_num[i].num, ((double) type_num[i].num / total_num) * 100.0);

  /* 打印匹配函数调用次数 */
  printf("\nNumber of Function Calls:\n\n");
  for (total_num = 0, i = 0; i < MATCH_FUN_NUM; i++)
    total_num += fun_calls[i].num;
  qsort(fun_calls, MATCH_FUN_NUM, sizeof(Str_Num_t), str_num_cmp);
  digits = get_digits(fun_calls[MATCH_FUN_NUM-1].num); /* 最大数的位数 */
  /* 从大到小输出 */
  for (i = TYPE_NUM - 1; i >= 0; i--)
    printf("%-20s : %*u (%5.2f%%)\n", fun_calls[i].name, digits, fun_calls[i].num, ((double) fun_calls[i].num / total_num) * 100.0);

  /* 打印不同长度map的数量 */
  printf("\nNumber of Map with Different Size:\n\n");
  for (total_num = 0, i = 1; i < ALPHABET_SIZE; i++)
    total_num += map_size[i].num_2;
  qsort(map_size, ALPHABET_SIZE, sizeof(Num_Num_t), num_num_cmp);
  digits = get_digits(map_size[ALPHABET_SIZE-1].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (i = ALPHABET_SIZE - 1; num = map_size[i].num_2; i--)
    if ((percent = ((double) num / total_num) * 100.0) > MIN_PERCENT)
      printf("%2u: %*u (%5.2f%%)\n", map_size[i].num_1, digits, num, percent);
  
  /* 打印不同长度数组的数量 */
  printf("\nNumber of Array with Different Size:\n\n");
  for (total_num = 0, i = 1; i < NUM_TO_BUILD_ARRAY + 1; i++)
    total_num += array_size[i].num_2;
  qsort(array_size, NUM_TO_BUILD_ARRAY + 1, sizeof(Num_Num_t), num_num_cmp);
  digits = get_digits(array_size[NUM_TO_BUILD_ARRAY].num_2); /* 最大数的位数 */
  /* 从大到小打印 */
  for (i = NUM_TO_BUILD_ARRAY; num = array_size[i].num_2; i--)
    if ((percent = ((double) num / total_num) * 100.0) > MIN_PERCENT)
      printf("%2u: %*u (%5.2f%%)\n", array_size[i].num_1, digits, num, percent);
}

#endif
