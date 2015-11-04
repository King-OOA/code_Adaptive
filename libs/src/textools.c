#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "textools.h"
#include "common.h"

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

    ifile = Fopen(iname,"r");
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
    ifile = Fopen(filename,"r");
    
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
