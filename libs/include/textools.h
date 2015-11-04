#ifndef TEXTOOLS_H
#define TEXTOOLS_H

#include <stdlib.h>
#include <stdio.h>


int get_file_alphabet(char const *filename);
size_t  get_file_size(FILE *fp);
void cut_file(char const *iname, int n);
int alphabet_compact(unsigned char *text, int text_len);

#endif
