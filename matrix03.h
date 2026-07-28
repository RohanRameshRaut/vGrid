#ifndef __MATRIX03_H_
#define __MATRIX03_H_
#include <ncurses.h>
#include "types.h"

void cell(WINDOW *local_win, Matrix m[3]);
void set_number(WINDOW *local_win, A_Matrix m2, Matrix m[3]);
void update_grid_content(WINDOW *local_win, Matrix m[3], Tcolor *head);

#endif
