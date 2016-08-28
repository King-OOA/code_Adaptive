#pragma once

#include "share.h"
#include <stdint.h>

typedef struct filter {
     Tree_Node_T children;
     int8_t block_size;
     int8_t window_size;
     Bitmap_T bitmap[];
} *Filter_T;

Filter_T build_filter(Suf_Node_T pat_list);
