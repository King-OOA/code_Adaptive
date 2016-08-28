OBJS = share.o hash.o map.o array.o statistics.o sorter.o filter.o
LIBS = -ladt -lbits -lcommon -lmem -lpatset -lm

#OBJS := $(patsubst ./src/%.c, %.o, $(wildcard ./src/*.c))
CC = gcc

INCLUDE_1 = ../libs/include
INCLUDE_2 = ./include

vpath %.h $(INCLUDE_1) $(INCLUDE_2)
vpath %.c ./src

CFLAGS = -Wall -c -std=c99 -O3 -I$(INCLUDE_1) -I$(INCLUDE_2)

ad_node_merge: main.o $(OBJS) $(LIBS)
	$(CC) $^ -o $@
	rm *.o

$(OBJS): share.h mem.h

main.o: main.c hash.h common.h map.h bits.h sorter.h array.h filter.h
	$(CC) $(CFLAGS) $< -o $@


$(OBJS): %.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: ra
ra : ro
	-rm a.test.out
ro:
	-rm $(OBJS)
