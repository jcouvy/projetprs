SRC := $(wildcard *.c)
CFLAGS := -std=c99 -Wall -g


maputils:  $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

bin:
	mkdir bin

clean:
	rm -fr bin
