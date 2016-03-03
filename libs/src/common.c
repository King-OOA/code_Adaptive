#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include "common.h"
//#include "share.h"

unsigned char buf[1024*1024]; /* 1MB buffer */

/* 截取文件iname的前 n MB 个字节 */
void cut_file(char const *iname, int n)
{
    char oname[1024];
    FILE *ifile,*ofile;

    if (n <= 0) {
	fprintf(stderr,"So <mb> must be positive!\n");
	exit(EXIT_FAILURE);
    }

    ifile = Fopen(iname,"rb");
    sprintf(oname,"%s.%iMB",iname,n);     /* 输出文件名, 和输入文件同属一个目录*/
    ofile = Fopen(oname,"w");
    
    while (n--) {
	if (fread (buf,1024*1024,1,ifile) != 1) { /* 每次读1MB */
	    fprintf(stderr,"Error: cannot read %s\n",iname);
	    fprintf(stderr, "%s\n", strerror(errno));
	    if (errno == 0)
		fprintf(stderr,"Maybe the file is too short?\n");
	    Fclose(ifile); Fclose(ofile);
	    unlink(oname); 	/* 删除oname文件*/
	    exit(EXIT_FAILURE);
	}
	if (fwrite (buf,1024*1024,1,ofile) != 1) { /* 每次写1MB */
	    fprintf(stderr,"Error: cannot write %s\n",oname);
	    fprintf(stderr, "%s\n", strerror(errno));
	    Fclose(ifile); Fclose(ofile);
	    unlink(oname);
	    exit(EXIT_FAILURE);
	}
    }

    Fclose(ifile);Fclose(ofile);
    
    printf("Successfully created %s.\n",oname);
}

/* 返回文本长度 */
size_t  get_file_size(FILE *fp)
{
    size_t size;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    return size;
}

/* 计算文件的字符集大小 */
int get_file_alphabet(char const *filename)
{
    int i, n;
    FILE *ifile;
    size_t sigma[256] = {0};
    ifile = Fopen(filename,"rb");
    
    /* 分块读取文件, 并累加各字符数 */
    while (n = fread(buf, 1, 1024*1024, ifile))
        while (n--)
            sigma[buf[n]]++;

    /* 统计各字符的出现次数 */
    for (i = 0; i < 256; i++) 
        if (sigma[i]) {
            n++;
            printf (" %c: %lu", i, sigma[i]);
        }
    
    printf ("\n\nFile %s has %i different characters\n", filename, n); 

    Fclose(ifile);
    
    return n;
}

/* 将文本的字符集压缩成连续的,返回新字符集的大小 */
int alphabet_compact(unsigned char *text, int text_len)
{
    unsigned char sigma[256] = { 0 };
    int i, j;

    for (i = 0; i < text_len; i++)
	sigma[text[i]] = 1;

    for (j = i = 0; i < 256; i++)
	if (sigma[i])
	    sigma[i] = ++j;

    for (i = 0; i < text_len; i++)
	text[i] = sigma[text[i]];
    
    return j;
}

char *str_cat(const char *str1, const char *str2)
{
    char *new_str = MALLOC(strlen(str1) + strlen(str2) + 1, char);

    return strcat(strcpy(new_str, str1), str2);
}

/* srand()在调用函数中使用,而非在rand_range中使用 */
int rand_range(int low, int high)
{
    double d = (double) rand() / ((double) RAND_MAX + 1);
    int k = (int) (d * (high - low + 1));

    return low + k;
}

void *mlloc(size_t size, const char *file, const char *fun, int line)
{
    void *new_mem = NULL;

    if (!(new_mem = malloc(size))) {
        fprintf(stderr, "Out of memory! function: %s, file:%s, line:%d\n", fun, file, line);
        exit(EXIT_FAILURE);
    }

    return new_mem;
}

void *clloc(size_t num, size_t size, const char *file, const char *fun, int line)
{
    void *new_mem = NULL;

    if (!(new_mem = calloc(num, size))) {
        fprintf(stderr, "Out of memory! function: %s, file:%s, line:%d\n", fun, file, line);
        exit(EXIT_FAILURE);
    }

    return new_mem;
}

/* 带错误处理的fopen */
FILE *Fopen(const char *name, const char *mode)
{
    FILE *fp;
    
    if ((fp = fopen(name, mode)) == NULL) {
        perror(name);
        exit(EXIT_FAILURE);
    }
     
    return fp;
}

/* 将名为file_name的文件载入内存,返回buffer,文件长度保存在file_size中 */
char *load_file(const char *file_name, size_t *file_size_p)
{
  FILE *fp = Fopen(file_name, "rb");
  char *buf;
  size_t file_size;

  file_size = get_file_size(fp);
  buf = MALLOC(file_size + 1, char);
  fread(buf, file_size, 1, fp);
  buf[file_size] = '\0';
 
  *file_size_p = file_size;

  return buf;
}

/* 带错误处理的fclose */
void Fclose(FILE *fp)
{
    if (fclose(fp)) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
}
