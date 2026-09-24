CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS = -I..
LDFLAGS = -L..
LDLIBS = -lhashmap

TEST_SOURCES := $(wildcard *.c)
TEST_PROGRAMS := $(TEST_SOURCES:.c=)

.PHONY: all run clean

all: $(TEST_PROGRAMS)

%: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

run: all
	@for test in $(TEST_PROGRAMS); do \
		echo "Running $$test"; \
		LD_LIBRARY_PATH=.. ./$$test || exit 1; \
	done

clean:
	rm -f $(TEST_PROGRAMS)
