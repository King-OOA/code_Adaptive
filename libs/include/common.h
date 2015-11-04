#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define MALLOC(num,type) (type *)mlloc((num)*sizeof(type), __FILE__, __FUNCTION__, __LINE__)
#define VMALLOC(struct_type,array_type,array_size) mlloc(sizeof(struct_type) + (array_size)*sizeof(array_type), __FILE__, __FUNCTION__, __LINE__)
#define CALLOC(num,type) (type *)clloc(num, sizeof(type), __FILE__, __FUNCTION__, __LINE__)

void *clloc(size_t, size_t, const char *, const char *, int);
void *mlloc(size_t, const char *, const char *, int);
FILE *Fopen(const char *, const char *);
void Fclose(FILE *);

char *str_cat(const char *, const char *);
int rand_int(int, int);

#endif
