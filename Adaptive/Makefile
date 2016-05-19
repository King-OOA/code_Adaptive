OBJS = main.o share.o hash.o map.o array.o statistics.o sorter.o
LIBS = -ladt -lcommon -lm -lbinary 

#OBJS := $(patsubst %.c, %.o, $(wildcard ./src/*.c))
CC = gcc

#LIBPATH = ../libs/so

INCLUDE_1 = ../libs/include
INCLUDE_2 = ./include

vpath %.h $(INCLUDE_1) $(INCLUDE_2)
vpath %.c ./src
#vpath lib%.a $(LIBPATH)
#vpath lib%.so $(LIBPATH)

CFLAGS = -Wall -c -std=c99 -O2 -I$(INCLUDE_1) -I$(INCLUDE_2)

a:  $(OBJS) $(LIBS)
	$(CC) $^ -o $@
	rm *.o

$(OBJS): share.h

main.o: main.c hash.h common.h map.h binary.h sorter.h array.h
	$(CC) $(CFLAGS) $< -o $@

share.o: share.c common.h
	$(CC) $(CFLAGS) $< -o $@

hash.o: hash.c hash.h common.h
	$(CC) $(CFLAGS) $< -o $@

map.o: map.c map.h common.h
	$(CC) $(CFLAGS) $< -o $@

array.o: array.c array.h common.h
	$(CC) $(CFLAGS) $< -o $@	

statistics.o: statistics.c statistics.h
	$(CC) $(CFLAGS) $< -o $@	

sorter.o : sorter.c sorter.h
	$(CC) $(CFLAGS) $<  -o $@


.PHONY: ra
ra :
	-rm a.test.out
