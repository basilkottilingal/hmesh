#SOURCE	= common.c mempool.c hmesh.c
SOURCE	= common.c 
PARENT := $(CURDIR)/..
INCLUDE = $(PARENT)/include/
OBJPATH = $(PARENT)/obj/

CC99		= gcc -std=c99
CFLAGS += -O2 -Wall -Wextra
CFLAGS += -I $(INCLUDE)	

%.o: %.c
	mkdir -p $(OBJPATH)
	$(CC99) $(CFLAGS) -c $< -o $(OBJPATH)/$@

OBJECTS = $(SOURCE:.c=.o)

#libhmesh.a: $(OBJECTS)

all: OBJECTS
