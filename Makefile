CC = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Werror
LDFLAGS = -shared
VERSION = 0.0.1
VERSION_MAJOR = $(word, $(VERSIOM))

LIB = libhashmap.so
SRC = hashmap.c
OBJ = $(SRC:.c=.o)

.PHONY: all test clean

all: $(LIB).$(VERSION)

$(LIB).$(VERSION): $(OBJ)
	$(CC) $(LDFLAGS) -fPIC -o $@ -Wl,-soname,$(LIB).0 $^
	ln -sf $(LIB).$(VERSION) $(LIB).$(VERSION_MAJOR)
	ln -sf $(LIB).$(VERSION_MAJOR) $(LIB)

test: $(LIB).$(VERSION)
	$(MAKE) -C tests
	$(MAKE) -C tests run

clean:
	$(MAKE) -C tests clean
	rm -f $(OBJ) $(LIB).$(VERSION) $(LIB).$(VERSION_MAJOR) $(LIB)
