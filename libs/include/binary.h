#pragma once 

#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

typedef unsigned char *byte_p;

void show_bytes(byte_p start, int len);
bool is_little_endian(void);

unsigned char *create_bitmap(size_t size);
void clear_bit(unsigned char *bitmap, uint64_t i);
void set_bit(unsigned char *bitmap, uint64_t i);
bool test_bit(const unsigned char *bitmap, uint64_t i);
bool is_odd(long n);
bool is_even(long n);
int logn(unsigned long n);
