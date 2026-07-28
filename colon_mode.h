#ifndef COLON_MODE_H
#define COLON_MODE_H

#include <ncurses.h>
#include <stdio.h>
#include "types.h"

int parse_int(const char *str, int *i);
int match_cmd(char *ucmd);
void err_msg(WINDOW *win, A_Matrix *m2, int index);
int ask_question_cmd(WINDOW *win, A_Matrix *m2, int index);
void change_color(Tcolor **head, int r, int c, unsigned char id);
int check_and_get_overlap(WINDOW *win, A_Matrix *m2, Tcolor **head, int start_row, int end_row, int start_col, int end_col, unsigned char *out_overlap);
void delete_cell(Tcolor **head, int row, int col);
Tcolor *find_cell(Tcolor *head, int row, int col);
int is_node_available(Tcolor **head, int row, int col);

void handle_resize_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, int rows, int cols);
void handle_exit_cmd(WINDOW *win, FILE *f);
void handle_goto_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor *head, int rows, int cols);
void handle_save_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, FILE *f, Tcolor **head);
void handle_color_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, int rows, int cols, int *str_idx);
void handle_copy_move_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int cols, int index, int *str_idx);
void handle_delete_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int cols, int *str_idx);
void handle_template_cmd(WINDOW *win, A_Matrix *m2, Tcolor **head, FILE *f);
void color_node(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int dst_rows, int cols, int dst_cols, int color_id, Cursor *c1);



void colon_mode(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, FILE *f);

#endif
