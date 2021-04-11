
all : test test2
test: ../my_vm.h
	gcc -g test.c -L../ -lmy_vm -m32 -o test

test2: test2.c
	gcc -g test2.c -L../ -lmy_vm -m32 -o test2 -W

clean:
	rm -rf test test2
