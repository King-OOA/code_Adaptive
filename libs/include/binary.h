#ifndef BINARY_H
#define BINARY_H
#include <stdio.h>

typedef unsigned char *byte_p;

void show_bytes(byte_p start, int len);
int is_little_endian(void);

unsigned char *create_bitmap(size_t size);
void clear_bit(unsigned char *bitmap, unsigned i);
void set_bit(unsigned char *bitmap, unsigned i);
int test_bit(const unsigned char *bitmap, unsigned i);
int is_odd(long n);
int is_even(long n);
int logn(unsigned long n);

#endif 
