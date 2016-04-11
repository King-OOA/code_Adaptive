#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include "common.h"
#include "makedata.h"
#include "stack.h"
#include "queue.h"
//#include "BST.h"
#include <limits.h>
//#include "lists.h"
#include "binary.h"
#include "arith.c"
#include <stdint.h>
#include <ctype.h>

#undef malloc

void bitmap_test(void);

int div16(int x)
{
  int bias = x >> 31 & 0xF;
  return (x + bias) >> 4;

}

struct b {
  int value;
  int key;
};


struct a {
  int len;
  struct b array[0];
};


void test()
{
  int local;

  printf("%p\n", &local);
}

int linenum;

int get_word(FILE *fp, char *buf, int size)
{
     int i, c;

     for (c = getc(fp); c != EOF && isspace(c); c = getc(fp))
	  if (c == '\n')
	       linenum++;   
     
     for (i = 0; c != EOF && !isspace(c); c = getc(fp))
	  if (i < size - 1)
	       buf[i++] = c;

     if (i < size) buf[i] = '\0';

     if (c != EOF) ungetc(c, fp);

     return buf[0] != '\0';
}

void double_word(char *file_name, FILE *fp)
{
     char prev[128], word[128];
     
     linenum = 1;
     prev[0] = '\0';

     while (get_word(fp, word, sizeof(word))) {
	  if (isalpha(word[0]) && strcmp(word, prev) == 0) {
	       if (file_name != NULL)
		    printf("%s:", file_name);
	       printf("%d: %s\n", linenum, word);
	  }
	  strcpy(prev, word);
     }
}

int main(int argc, char **argv)
{
     int i = 1;
     FILE *fp;

     while (argv[i] != NULL) {
	  fp = Fopen(argv[i], "r");
	  double_word(argv[i], fp);
     }

     if (argc == 1)
	  double_word(NULL, stdin);
}


  //  printf("key:%d, value: %d\n", p->array[1].key, p->array[8].value);
  //printf("key:%d, value: %d\n", p->array[1].key, p->array[8].value);
  
  
  //str.p = S;
  



