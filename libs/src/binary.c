#include <stdio.h>
#include "binary.h"
#include "common.h"

#define BIT_PER_CHAR (sizeof(unsigned char) * 8)
#define SHIFT 3
#define MASK 0x7


/* 生成size位的bitmap */
unsigned char *create_bitmap(size_t size)
{
  unsigned char *bitmap = CALLOC(size / BIT_PER_CHAR + 1, unsigned char);
  
  return bitmap;
}

/* 将bitmap的第i位置(i从0开始)1 */
void set_bit(unsigned char *bitmap, unsigned i)
{
  bitmap[i>>SHIFT] |= (1 << (i & MASK));
}

/* 将bitmap的第i位置0 */
void clear_bit(unsigned char *bitmap, unsigned i)
{
  bitmap[i>>SHIFT] &= ~(1 << (i & MASK));
}

/* 测试bitmap的第i位是否为1 */
int test_bit(unsigned char const *bitmap, unsigned i)
{
  return bitmap[i>>SHIFT] & (1 << (i & MASK));
}

/* 一个对象的字节表示,start是该对象的起始地址,len为该对象的字节数 */
void show_bytes(byte_p start, int len)
{
    int i;
    
    for (i = 0; i < len; i++)
        printf(" %.2x", start[i]);
    putchar('\n');
}

/* 以2为底n的对数, n必须是2的幂 */
int logn(unsigned long n)
{
  int i;

  for (i = 0; n >>= 1; i++)
    ;

  return i;
}


/* 判断n是否是奇数 */
int is_odd(long n)
{
  return n & 0x1;
}

/* 判断n是否是偶数 */
int is_even(long n)
{
  return !(n & 0x1);
}

/* 小端机器返回1, 大端返回0 */
int is_little_endian(void)
{
    int x = 1;
    
    return *((unsigned char*) &x);
}


