SRC := $(wildcard *.c)
CFLAGS := -std=c99 -Wall -g


all:  $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

bin:
	mkdir bin

clean:
	rm -fr bin
