#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "mem.h"

#include "filter.h"
#include "share.h"
#include "common.h"

static uint32_t pow_256[] = {1, 256, 256*256, 256*256*256};

static Filter_T filter_new(int8_t block_size, Pat_Len_T lss)
{
     uint32_t n = pow_256[block_size];
     Filter_T new_filter;
     VNEW(new_filter, n, Bitmap_T);

     new_filter->block_size = block_size;
     new_filter->window_size = lss;

     new_filter->children = CALLOC(n, struct Tree_Node);
     memset(new_filter->bitmap, 0, n * sizeof(Bitmap_T));

     return new_filter;
}

Pat_Len_T get_lss(Suf_Node_T suf_list);
extern struct Suf_Node ***make_tails(Pat_Num_T table_size, Tree_Node_T child);

Filter_T build_filter(Suf_Node_T pat_list)
{
     Pat_Len_T lss = get_lss(pat_list); printf("LSP: %d\n", lss);
     int8_t block_size = (lss == 1) ? 1 : 2;
     Filter_T filter = filter_new(block_size, lss);
     uint32_t n = pow_256[block_size];
     
     printf("\nBuilding filter...\n");
     struct Suf_Node ***tails = make_tails(n, filter->children);

     for (Suf_Node_T cur_suf = pat_list, next_suf; cur_suf; cur_suf = next_suf) {
	  next_suf = cur_suf->next;
	  cur_suf->next = NULL;
	  uint32_t v;
	  for (Pat_Len_T i = 0; i <= lss - block_size; i++) {
	       v = block_123(cur_suf->str + i, block_size);
	       set_bit(filter->bitmap + v, i);
	  }
	  *tails[v] = cur_suf; tails[v] = &cur_suf->next;
     }
     
     free(tails);
     push_children(filter->children, n);
     
     /* 末尾为1的bitmap的比例 */
     uint32_t num_of_1 = 0;
     for (uint32_t i = 0; i < n; i++)
       if (test_bit(filter->bitmap + i, lss - block_size))
	   num_of_1++;
     printf("Occupied: %.2f\n", (double) num_of_1 / n);

     return filter;
}

