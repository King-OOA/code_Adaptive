SUF = so
CC = gcc

LINKSO = gcc -shared
LINKA = ar -rc
LINK = $(LINKSO)

INCLUDE = ./include
CFLAGS = -g -Wall -c -std=c99 -fPIC -I$(INCLUDE)

#APATH = ./a
#SOPATH = ./so
LIBPATH = /usr/local/lib/

vpath %.h $(INCLUDE) ../Adaptive/include
vpath %.c ./src
#vpath %.so $(SOPATH)
#vpath %.a $(APATH)

#OBJS := $(patsubst %.c,%.o, $(wildcard ./sc/*.c))


SOS = libcommon.so libbinary.so libadt.so
LIBS = $(SOS) #-ladt -lcommon -lm -lbinary 

a.out: main.o $(SOS)
	$(CC) $(CFLAGS) -ladt -lcommon -lm -lbinary -o $@ 
	rm *.o	

$(OBJS): common.h

main.o: main.c makedata.h stack.h binary.h queue.h
	$(CC) $(CFLAGS) $< -o $@

libcommon.$(SUF): common.o
	$(LINK) $^ -o $(LIBPATH)$@
common.o: common.c
	$(CC) $(CFLAGS) $< -o $@

# libmakedata.$(SUF): makedata.o
# 	$(LINK) $^ -o $(LIBPATH)/$@
# makedata.o : makedata.c makedata.h
# 	$(CC) $(CFLAGS) $< -o $@

libbinary.$(SUF): binary.o
	$(LINK) $^ -o $(LIBPATH)$@
binary.o: binary.c binary.h
	$(CC) $(CFLAGS) $< -o $@


libadt.$(SUF): queue.o arith.o
	$(LINK) $^ -o $(LIBPATH)$@
#stack.o: stack.c stack.h
#	$(CC) $(CFLAGS) $< -o $@
queue.o: queue.c queue.h
	$(CC) $(CFLAGS) $< -o $@
#lists.o: lists.c lists.h
#	$(CC) $(CFLAGS) $< -o $@
arith.o : arith.c arith.h
	$(CC) $(CFLAGS) $< -o $@


.PHONY: clean ro rso ra
clean : ro rso ra
	-rm a.out
ro :
	-rm *.o
rso:
	-rm $(SOPATH)/*.so
ra:
	-rm $(APATH)/*.a
