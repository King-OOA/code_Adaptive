#pragma once

#include "share.h"
#include <stdint.h>


struct num_num {	/* 统计元素 */
  uint32_t num_1;
  uint32_t num_2;
};

struct str_num {	/* 统计元素 */
  char name[100];
  uint32_t num;
};

typedef struct num_num Num_Num_T;
typedef struct str_num Str_Num_T;

void print_statistics(void);
