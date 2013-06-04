CFLAGS += -g

all: test

test: test.o

test.o: test.c

.PHONY: clean
clean:
	rm -rf test *.o
