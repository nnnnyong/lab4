CC := gcc
CFLAGS := -Wall -Wextra -g
GTKFLAGS := $(shell pkg-config --cflags --libs gtk+-3.0)
SRC := $(wildcard *.c)
EXE := $(patsubst %.c,%,$(SRC))

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(GTKFLAGS)

clean:
	rm -f $(EXE)

