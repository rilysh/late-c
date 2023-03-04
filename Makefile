CC ?= gcc
FLAGS = -Wall -O2 -s
PROG = late

# Or just run gcc/clang late.c
all:
	$(CC) $(FLAGS) $(PROG).c -o $(PROG)

clean:
	rm $(PROG)
