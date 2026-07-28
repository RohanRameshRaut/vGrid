#ifndef __NAVIGATION_H_
#define __NAVIGATION_H_
#include<ncurses.h>
#include "types.h"

void go_left(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head);
void go_right(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head);
void go_up(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head);
void go_down(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head);
void go_to(int rows, int cols, WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, Tcolor *head);

#endif

