CC=gcc
CFLAGS=-Wall -g

# default target (ie. "make all" = "make")
# check if "librarytest" changes (ie. check if any if "librarytest" dependecies change)
all: librarytest

# create executable, highest level needs tester and library
# $@ = librartest, $^ = libmycode.o
librarytest: libmycode.o librarytest.c
	$(CC) $(CFLAGS) -o $@ $^

# create library/.o file
libmycode.o: libmycode.c mycode.h
	$(CC) $(CFLAGS) -c libmycode.c

# will run since no dependencies
clean:
	rm -rf libmycode.o librarytest 

