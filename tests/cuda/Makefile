PARENT := $(CURDIR)/../..
INCDIR	= $(PARENT)/include
SRCDIR	= $(PARENT)/src
OBJDIR	= $(PARENT)/obj

CC99		= gcc -std=c99

#CFLAGS += -O2 -Wall -Wextra -D_MANIFOLD_DEBUG
CFLAGS += -I $(INCDIR)

%.tst: %.cu 
	cd $(SRCDIR) && make libhmesh.a 
	nvcc $(CFLAGS) $< $(SRCDIR)/libhmesh.a -o run

