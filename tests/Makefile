PARENT := $(CURDIR)/..
INCDIR	= $(PARENT)/include
SRCDIR	= $(PARENT)/src
OBJDIR	= $(PARENT)/obj

CC99		= gcc -std=c99

CFLAGS += -O2 -Wall -Wextra -D_MANIFOLD_DEBUG
CFLAGS += -I $(INCDIR)

%.tst: %.c 
	cd $(SRCDIR) && make libhmesh.a 
	$(CC99) $(CFLAGS) $*.c $(SRCDIR)/libhmesh.a -o run

