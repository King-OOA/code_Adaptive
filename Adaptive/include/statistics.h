#ifndef STATISTICS_H
#define STATISTICS_H

#include "share.h"

typedef struct num_num_elmt {	/* 统计元素 */
  UC_t num_1;
  unsigned num_2;
} Num_Num_t;


typedef struct str_num_elmt {	/* 统计元素 */
  char name[100];
  unsigned num;
} Str_Num_t;

void print_statistics(Bool_t show_details);

#endif 
