SOURCE	= common.c 
OBJPATH = ../obj/

CC99		= gcc -std=c99

CFLAGS += -O2 -Wall -Wextra
CFLAGS += -I $(INCLUDE)	

%.o: %.c
	mkdir -p $(OBJPATH)
	$(CC99) $< $(CFLAGS) -c -o $(OBJPATH)/$@

OBJECTS = $(SOURCE:.c=.o)

all: OBJECTS
