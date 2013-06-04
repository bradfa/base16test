CFLAGS += -g -Wall -Werror -Wextra

all: test

test: test.o

test.o: test.c

.PHONY: clean
clean:
	rm -rf test *.o
