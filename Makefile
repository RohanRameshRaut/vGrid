CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lncurses

_ite: grid_v07.c template_parser.c read_file.c write_file.c _initialise.c colon_mode.c navigation.c matrix03.c node_list.c
	$(CC) $(CFLAGS) grid_v07.c template_parser.c read_file.c write_file.c _initialise.c colon_mode.c navigation.c matrix03.c node_list.c -o _ite $(LIBS)

clean:
	rm -f _ite

.PHONY: clean
