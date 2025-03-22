SOURCE	= common.c mempool.c tree-pool.c hmesh.c
PARENT := $(CURDIR)/..
INCDIR	= $(PARENT)/include
OBJDIR	= $(PARENT)/obj

CC99		= gcc -std=gnu99
CFLAGS += -O2 -Wall -Wextra
CFLAGS += -I $(INCDIR) 

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	$(CC99) $(CFLAGS) -c $< -o $@

OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(SOURCE))

libhmesh.a: $(OBJECTS) Makefile
	ar -cr $@ $^

all: libhmesh.a

clean:
	rm -r $(OBJDIR)
	rm libhmesh.a 
