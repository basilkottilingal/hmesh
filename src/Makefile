SOURCE	= common.c mempool.c
PARENT := $(CURDIR)/..
INCDIR  = $(PARENT)/include
OBJDIR  = $(PARENT)/obj

CC99		= gcc -std=c99
CFLAGS += -O2 -Wall -Wextra
CFLAGS += -I $(INCDIR)	

$(OBJDIR)/%.o: %.c
	mkdir -p $(OBJDIR)
	$(CC99) $(CFLAGS) -c $< -o $@

OBJECTS = $(patsubst %.c, $(OBJDIR)/%.o, $(SOURCE))

libhmesh.a: $(OBJECTS)
	ar -cr $@ $^ 

all: libhmesh.a
