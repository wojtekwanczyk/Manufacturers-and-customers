
CC = gcc
CFLAGS = -g -O2 -std=gnu99 -Wall -pthread -lrt


all: main

main: main.c
	$(CC) $(CFLAGS) main.c -o main

clean:
	rm -f *.o main


test1:
	./main 10 20 5 categories.txt 20 \> on 0

test2:
	./main 10 20 5 tadeusz.txt 20 \> on 0

test3:
	./main 10 20 10 tadeusz.txt 20 \> off 1


