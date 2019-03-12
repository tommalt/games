.PHONY: clean

CC=gcc
CFLAGS=-g -ggdb -std=c99 `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2`

SRC=snake.c
OBJ=$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

a.out: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

clean:
	@echo cleaning
	@rm -f *.o a.out

