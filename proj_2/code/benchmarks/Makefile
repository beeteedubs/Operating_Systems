CC = gcc
CFLAGS = -g -w

all:: parallel_cal vector_multiply external_cal test

parallel_cal:
	$(CC) $(CFLAGS) -pthread -o parallel_cal parallel_cal.c -L../ -lrpthread

vector_multiply:
	$(CC) $(CFLAGS) -pthread -o vector_multiply vector_multiply.c -L../ -lrpthread

external_cal:
	$(CC) $(CFLAGS) -pthread -o external_cal external_cal.c -L../ -lrpthread

test:
	$(CC) $(CFLAGS) -pthread -o test test.c -L../ -lrpthread -g

test2:
	$(CC) $(CFLAGS) -pthread -o test2 test2.c -L../ -lrpthread -g

clean:
	rm -rf testcase test test2  parallel_cal vector_multiply external_cal *.o ./record/
