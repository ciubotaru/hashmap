CC = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Werror
LDFLAGS = -shared

LIB = libhashmap.so
SRC = hashmap.c
OBJ = $(SRC:.c=.o)

.PHONY: all test clean

all: $(LIB)

$(LIB): $(OBJ)
	$(CC) $(LDFLAGS) -fPIC -o $@ $^

test: $(LIB)
	$(MAKE) -C tests
	$(MAKE) -C tests run

clean:
	$(MAKE) -C tests clean
	rm -f $(OBJ) $(LIB)
