SOURCE	= ast.c memory.c hash.c

CC99		= gcc -std=c99
CFLAGS += -O2 -Wall -Wextra -I.

%.o: %.c %.h
	$(CC99) $(CFLAGS) -c $< -o $@

OBJECTS = $(patsubst %.c, %.o, $(SOURCE))

libast.a: $(OBJECTS) Makefile
	ar -cr $@ $^

all: libast.a

clean:
	rm $(OBJECTS)
	rm libhmesh.a 
