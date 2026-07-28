#include <stdlib.h>
#include <ncurses.h>
#include "colon_mode.h"
#include "types.h"
#include "_initialise.h"
#include "matrix03.h"
#include "navigation.h"
#include "node_list.h"    
#include "write_file.h"
#include "read_file.h"
#include "template_parser.h"

int parse_int(const char *str, int *i){
	int num = 0;
	while (str[*i] == ' ') {
		(*i)++;
	}
	while (str[*i] >= '0' && str[*i] <= '9') {
		num = num * 10 + (str[*i] - '0');
		(*i)++;
	}
	return num;
}

int match_cmd(char *ucmd){
	char *cmd[] = CMD;
	for (int i = 0; i < CMD_LEN; i++) {
		int j = 0;
		while (ucmd[j] != ' ' && ucmd[j] != '\0' && cmd[i][j] != '\0') {
			if (cmd[i][j] != ucmd[j]) {
				break;
			}
			j++;
		}
		if (cmd[i][j] == '\0' && (ucmd[j] == ' ' || ucmd[j] == '\0')) {
			return i;
		}
	}
	return -1;
}

void err_msg(WINDOW *win, A_Matrix *m2, int index){
	char *err[] = ERR_MSG;
	wattron(win, COLOR_PAIR(1));
	mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x, "%s", err[index]);
	wattroff(win, COLOR_PAIR(1));
	wrefresh(win);
	napms(1500);
	wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
	wclrtoeol(win);
	wrefresh(win);
}

int ask_question_cmd(WINDOW *win, A_Matrix *m2, int index){
	char *err[] = QUESTIONS;
	mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x, "%s", err[index]);
	wrefresh(win);

	int ch;
	while (1) {
		ch = wgetch(win);
		if (ch == '0' || ch == '1') {
			wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
			wclrtoeol(win);
			wrefresh(win);
			return (ch - '0');
		}
		if (ch == 27) {
			wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
			wclrtoeol(win);
			wrefresh(win);
			return 0;
		}
	}
}

void change_color(Tcolor **head, int r, int c, unsigned char id){
	Tcolor *curr = *head;
	Tcolor *last_match = NULL;

	while (curr) {
		if (curr->y == r && curr->x == c) {
			last_match = curr;
		}
		curr = curr->next;
	}

	if (last_match){
		last_match->id = id;

		Tcolor *next = last_match->next;
		while (next && next->y == r && next->x == c) {
			Tcolor *temp = next;
			next = next->next;
			free(temp);
		}
		last_match->next = next;
	}
}

int check_and_get_overlap(WINDOW *win, A_Matrix *m2, Tcolor **head, int start_row, int end_row, int start_col, int end_col, unsigned char *out_overlap){
	int has_existing = 0;
	*out_overlap = 0;

	for (int r = start_row; r <= end_row; r++) {
		for (int c = start_col; c <= end_col; c++) {
			if (is_node_available(head, r, c)) {
				has_existing = 1;
				r = end_row + 1;
				break;
			}
		}
	}

	if (has_existing) {
		int overlap_mode = ask_question_cmd(win, m2, 0);
		if (overlap_mode) {
			*out_overlap = (unsigned char)ask_question_cmd(win, m2, 1);
			return 1;
		}
	}
	return 0;
}


void delete_cell(Tcolor **head, int row, int col) {
	if (!head || !*head) return;

	Tcolor *curr = *head;
	Tcolor *prev = NULL;

	Tcolor *target_node = NULL;
	Tcolor *target_prev = NULL;

	while (curr) {
		if (curr->y == row && curr->x == col) {
			target_node = curr;
			target_prev = prev;
		}
		prev = curr;
		curr = curr->next;
	}

	if (target_node) {
		if (target_prev) {
			target_prev->next = target_node->next;
		} else {
			*head = target_node->next;
		}
		free(target_node);
	}
}


Tcolor *find_cell(Tcolor *head, int row, int col){
	Tcolor *curr = head;
	Tcolor *last_match = NULL;
	while (curr) {
		if (curr->y == row && curr->x == col){
			last_match = curr;
		}
		curr = curr->next;
	}
	return last_match;
}

int is_node_available(Tcolor **head, int row, int col){
	return (find_cell(*head, row, col) != NULL);
}

void handle_resize_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, int rows, int cols){
	unsigned int arr[4];
	arr[0] = (unsigned int)rows;
	arr[1] = (unsigned int)cols;
	arr[2] = (unsigned int)m[3].rows;
	arr[3] = (unsigned int)m[3].cols;

	if (initialise_argv(m, m2, c1, arr)) {
		wclear(win);
		cell(win, m);
		update_grid_content(win, m, *head);
		set_number(win, *m2, m);
		wrefresh(win);
		wmove(win, c1->pcy, c1->pcx);
	} else {
		err_msg(win, m2, 0);
	}
}

void handle_exit_cmd(WINDOW *win, FILE *f){
	if (ff) fclose(ff);
	if (f) fclose(f);
	unlink("null.ite");

	delwin(win);
	endwin();
	exit(0);
}

void handle_goto_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor *head, int rows, int cols){
	if ((0 <= rows && rows <= m[3].rows) && (0 <= cols && cols <= m[3].cols)) {
		go_to(rows, cols, win, m, *m2, c1, head);
	} else {
		err_msg(win, m2, 1);
	}
}

void handle_save_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, FILE *f, Tcolor **head){
	if (!write_in_file(f, m, head)) {
		err_msg(win, m2, 7);
		return;
	}

	/* write_in_file() just rebased file_meta.template_offset to be
	 * relative to f. ff ("null.ite") still holds the OLD layout, so
	 * it must be re-synced to mirror f, or every template lookup
	 * after this point reads the wrong bytes. */
	fflush(f);
	if (fseek(f, 0, SEEK_SET) == 0) {
		if (ff) { fclose(ff); ff = NULL; }

		ff = fopen("null.ite", "wb+");
		if (ff) {
			char buf[4096];
			size_t n;
			while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
				fwrite(buf, 1, n, ff);
			}
			fflush(ff);
		} else {
			perror("fopen error");
		}
	}
	fseek(f, 0, SEEK_END);

	err_msg(win, m2, 8);
}


void handle_color_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, int rows, int cols, int *str_idx){
	int dst_rows = rows;
	int dst_cols = cols;

	if (m2->b1[++(*str_idx)] == '#' && m2->b1[*str_idx] != '\0') {
		(*str_idx)++;
	} else {
		dst_rows = parse_int(m2->b1, str_idx);
		dst_cols = parse_int(m2->b1, str_idx);
		(*str_idx)++;
	}

	if (m2->b1[*str_idx] == '\0') {
		err_msg(win, m2, 3);
		return;
	}

	char *hex_ptr = &m2->b1[*str_idx];
	color_palette(color_arr);
	uint8_t color_id = hex_to_rgb(color_arr, hex_ptr);

	if (rows <= m[3].rows && cols <= m[3].cols && dst_rows <= m[3].rows && dst_cols <= m[3].cols){
		color_node(win, m, m2, head, rows, dst_rows, cols, dst_cols, color_id, c1);
	}
	else {
		err_msg(win, m2, 1);
	}
}

void color_node(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int dst_rows, int cols, int dst_cols, int color_id, Cursor *c1){
	unsigned char overlap = 0;
	int has_overlap_choice = check_and_get_overlap(win, m2, head, rows, dst_rows, cols, dst_cols, &overlap);

	for (int r = rows; r <= dst_rows; r++) {
		for (int c = cols; c <= dst_cols; c++) {
			if (is_node_available(head, r, c)) {
				if (has_overlap_choice) {
					link_tcolor(head, color_id, c, r, overlap);
				} else {
					change_color(head, r, c, color_id);
				}
			} else {
				link_tcolor(head, color_id, c, r, 0);
			}
		}
	}
	go_to(rows, cols, win, m, *m2, c1, *head);
	update_grid_content(win, m, *head);
}

void handle_copy_move_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int cols, int index, int *str_idx){
	int srce_rows = parse_int(m2->b1, str_idx);
	int srce_cols = parse_int(m2->b1, str_idx);
	int dst_rows, dst_cols;

	if (m2->b1[*str_idx] == '\0') {
		dst_rows = srce_rows;
		dst_cols = srce_cols;
		srce_rows = rows;
		srce_cols = cols;
	} else {
		dst_rows = parse_int(m2->b1, str_idx);
		dst_cols = parse_int(m2->b1, str_idx);
	}

	int row_offset = dst_rows - rows;
	int col_offset = dst_cols - cols;

	if (((srce_rows - rows) + dst_rows > m[3].rows) || ((srce_cols - cols) + dst_cols > m[3].cols)) {
		err_msg(win, m2, 4);
		return;
	}

	unsigned char overlap = 0;
	int has_overlap_choice = check_and_get_overlap(win, m2, head, dst_rows, srce_rows + row_offset, dst_cols, srce_cols + col_offset, &overlap);

	Tcolor *temp_list = NULL;

	for (int r = rows; r <= srce_rows; r++) {
		for (int c = cols; c <= srce_cols; c++) {
			Tcolor *node = find_cell(*head, r, c);
			if (node) {
				link_tcolor(&temp_list, node->id, c + col_offset, r + row_offset, has_overlap_choice ? overlap : 0);
			}
		}
	}

	if (index == 5) {
		for (int r = rows; r <= srce_rows; r++) {
			for (int c = cols; c <= srce_cols; c++) {
				delete_cell(head, r, c);
			}
		}
	}

	if (!has_overlap_choice) {
		for (int r = dst_rows; r <= (srce_rows + row_offset); r++) {
			for (int c = dst_cols; c <= (srce_cols + col_offset); c++) {
				delete_cell(head, r, c);
			}
		}
	}

	Tcolor *curr_temp = temp_list;
	while (curr_temp) {
		Tcolor *next_node = curr_temp->next;
		curr_temp->next = NULL;

		merge_list(head, &curr_temp); 
		curr_temp = next_node;
	}

	update_grid_content(win, m, *head);
}
void handle_delete_cmd(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head, int rows, int cols, int *str_idx){
	int dst_rows = rows;
	int dst_cols = cols;

	if (m2->b1[*str_idx] != '\0') {
		dst_rows = parse_int(m2->b1, str_idx);
		dst_cols = parse_int(m2->b1, str_idx);

		if (dst_rows > m[3].rows || dst_cols > m[3].cols) {
			err_msg(win, m2, 4);
			return;
		}
	}

	for (int r = rows; r <= dst_rows; r++) {
		for (int c = cols; c <= dst_cols; c++) {
			delete_cell(head, r, c);
		}
	}
	update_grid_content(win, m, *head);
}

void handle_template_cmd(WINDOW *win, A_Matrix *m2, Tcolor **head, FILE *f){
	Template t;
	t.name = NULL;
	t.c = NULL;
	t.t = NULL;

	Template_parser template_parser;
	template_parser_function(m2->b1, &template_parser);

	int val = validate_template_parser_function(&t, template_parser, head);

	if(val == 2){
		err_msg(win, m2, 9);
	} else if(val == 3){
		err_msg(win, m2, 10);
	} else if(val == 1){
		err_msg(win, m2, 6);
	} else if(val == 4){
		err_msg(win, m2, 5);
	} else if(!val){
		if(save_template(&t, f)){
			fflush(ff);
			err_msg(win, m2, 11);
		} else{
			err_msg(win, m2, 12);
		}
		if (t.c) free(t.c);
		if (t.t) free(t.t);
	}

	if (template_parser.template_name) {
		free(template_parser.template_name);
	}
	if (template_parser.template_arr) {
		for (int k = 0; k < template_parser.template_cnt; k++) {
			if (template_parser.template_arr[k]) free(template_parser.template_arr[k]);
		}
		free(template_parser.template_arr);
	}
	if (template_parser.single_arr) {
		for (int k = 0; k < template_parser.single_arr_cnt; k++) {
			if (template_parser.single_arr[k]) free(template_parser.single_arr[k]);
		}
		free(template_parser.single_arr);
	}
	if (template_parser.range_arr) {
		for (int k = 0; k < template_parser.range_arr_cnt; k++) {
			if (template_parser.range_arr[k]) free(template_parser.range_arr[k]);
		}
		free(template_parser.range_arr);
	}
}

void extract_cells_from_template(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head,
		Cursor *c1, Template_file t, uint32_t base_offset,
		int dst_top_x, int dst_top_y,
		int has_overlap_choice, unsigned char overlap);

void generate_cells(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head,
		Cursor *c1, Template_file t, uint32_t base_offset,
		int dst_top_x, int dst_top_y,
		int has_overlap_choice, unsigned char overlap);


void handle_use_template_cmd(WINDOW *win, A_Matrix *m2, Matrix m[4], Tcolor **head,
		Cursor *c1, int dst_top_y, int dst_top_x, int *str_idx)
{
	while (m2->b1[*str_idx] == ' ' || m2->b1[*str_idx] == '\t') {
		(*str_idx)++;
	}

	char template_name[UBUF] = {0};
	int k = 0;

	while (m2->b1[*str_idx] != '\0' &&
			m2->b1[*str_idx] != ' '  &&
			m2->b1[*str_idx] != '\r' &&
			m2->b1[*str_idx] != '\n' &&
			k < UBUF - 1)
	{
		template_name[k++] = m2->b1[(*str_idx)++];
	}
	template_name[k] = '\0';

	if (k == 0) {
		err_msg(win, m2, 10);
		return;
	}

	uint32_t offset = search_template((const unsigned char *)template_name);

	if (offset == INVALID_OFFSET) {
		err_msg(win, m2, 10);
		return;
	}

	Template_file t;
	if (fseek(ff, offset, SEEK_SET) != 0) return;
	if (fread(&t, sizeof(Template_file), 1, ff) != 1) return;

	int dst_bottom_x = dst_top_x + (t.max_x - t.min_x);
	int dst_bottom_y = dst_top_y + (t.max_y - t.min_y);

	if (dst_top_x < 0 || dst_top_x > m[3].cols ||
			dst_top_y < 0 || dst_top_y > m[3].rows ||
			dst_bottom_x < 0 || dst_bottom_x > m[3].cols ||
			dst_bottom_y < 0 || dst_bottom_y > m[3].rows)
	{
		err_msg(win, m2, 13);
		return;
	}

	unsigned char overlap = 0;
	int has_overlap_choice = check_and_get_overlap(win, m2, head,
			dst_top_y, dst_bottom_y, dst_top_x, dst_bottom_x, &overlap);
	extract_cells_from_template(win, m, m2, head, c1, t, offset, dst_top_x, dst_top_y,
			has_overlap_choice, overlap);

	go_to(dst_top_y, dst_top_x, win, m, *m2, c1, *head);
	update_grid_content(win, m, *head);
}

void generate_cells(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head,
		Cursor *c1, Template_file t, uint32_t base_offset,
		int dst_top_x, int dst_top_y,
		int has_overlap_choice, unsigned char overlap)
{
	(void)win; (void)m2; (void)c1; (void)m;

	if (t.cell_count == 0) return;

	Tcolor_template *c = malloc(t.cell_count * sizeof(Tcolor_template));
	if (!c) return;

	fseek(ff, base_offset + t.c, SEEK_SET);
	if (fread(c, sizeof(Tcolor_template), t.cell_count, ff) == (size_t)t.cell_count) {
		for (int i = 0; i < t.cell_count; i++) {
			int target_y = dst_top_y + (c[i].y - t.min_y);
			int target_x = dst_top_x + (c[i].x - t.min_x);

			if (is_node_available(head, target_y, target_x)) {
				if (has_overlap_choice) {
					link_tcolor(head, c[i].id, target_x, target_y, overlap);
				} else {
					change_color(head, target_y, target_x, c[i].id);
				}
			} else {
				link_tcolor(head, c[i].id, target_x, target_y, 0);
			}
		}
	}
	free(c);
}

void extract_cells_from_template(WINDOW *win, Matrix m[4], A_Matrix *m2, Tcolor **head,
		Cursor *c1, Template_file t, uint32_t base_offset,
		int dst_top_x, int dst_top_y,
		int has_overlap_choice, unsigned char overlap)
{
	int count = t.temp_count;
	while (count > 0) {
		uint32_t child_offset;
		Template_file tt;

		fseek(ff, base_offset + t.t + (sizeof(uint32_t) * (count - 1)), SEEK_SET);
		if (fread(&child_offset, sizeof(uint32_t), 1, ff) == 1) {
			fseek(ff, child_offset, SEEK_SET);
			if (fread(&tt, sizeof(Template_file), 1, ff) == 1) {
				extract_cells_from_template(win, m, m2, head, c1, tt, child_offset,
						dst_top_x, dst_top_y, has_overlap_choice, overlap);
			}
		}
		count--;
	}

	generate_cells(win, m, m2, head, c1, t, base_offset, dst_top_x, dst_top_y,
			has_overlap_choice, overlap);
}


void colon_mode(WINDOW *win, Matrix m[4], A_Matrix *m2, Cursor *c1, Tcolor **head, FILE *f){
	wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
	wclrtoeol(win);
	mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x, "%c", COLON);
	noecho();

	int ch = 0;
	int index1 = 0;
	int cursor_pos = 0;
	m2->b1[0] = '\0';
	static int n = 0;
	int history_pos = n;

	while (1) {
		wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE + cursor_pos);
		ch = wgetch(win);

		if (ch == '\n' || ch == '\r') {
			m2->b1[index1] = '\0';
			if (n < UBUF && index1 > 0) {
				int is_duplicate = 0;
				if (n > 0) {
					int match_idx = 0;
					while (m2->b2[n-1][match_idx] != '\0' || m2->b1[match_idx] != '\0') {
						if (m2->b2[n-1][match_idx] != m2->b1[match_idx]) {
							is_duplicate = 0;
							break;
						}
						is_duplicate = 1;
						match_idx++;
					}
				}
				if (!is_duplicate) {
					int k;
					for (k = 0; m2->b1[k] != '\0'; k++) {
						m2->b2[n][k] = m2->b1[k];
					}
					m2->b2[n][k] = '\0';
					n++;
				}
			}
			break;
		}
		else if (ch == KEY_UP) {
			if (n > 0 && history_pos > 0) {
				history_pos--;
				wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE);
				wclrtoeol(win);
				int k = 0;
				while (m2->b2[history_pos][k] != '\0' && k < UBUF - 1) {
					m2->b1[k] = m2->b2[history_pos][k];
					k++;
				}
				m2->b1[k] = '\0';
				index1 = k;
				cursor_pos = k;
				mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE, "%s", m2->b1);
				wrefresh(win);
			}
		}
		else if (ch == KEY_DOWN) {
			if (history_pos < n - 1) {
				history_pos++;
				wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE);
				wclrtoeol(win);
				int k = 0;
				while (m2->b2[history_pos][k] != '\0' && k < UBUF - 1) {
					m2->b1[k] = m2->b2[history_pos][k];
					k++;
				}
				m2->b1[k] = '\0';
				index1 = k;
				cursor_pos = k;
				mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE, "%s", m2->b1);
				wrefresh(win);
			} else if (history_pos == n - 1) {
				history_pos = n;
				m2->b1[0] = '\0';
				index1 = 0;
				cursor_pos = 0;
				wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE);
				wclrtoeol(win);
				wrefresh(win);
			}
		}
		else if (ch == KEY_LEFT) {
			if (cursor_pos > 0) cursor_pos--;
		}
		else if (ch == KEY_RIGHT) {
			if (cursor_pos < index1) cursor_pos++;
		}
		else if (ch == KEY_BACKSPACE || ch == 8 || ch == 127) {
			if (cursor_pos > 0) {
				for (int k = cursor_pos - 1; k < index1; k++) {
					m2->b1[k] = m2->b1[k + 1];
				}
				index1--;
				cursor_pos--;
				wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE);
				wclrtoeol(win);
				mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE, "%s", m2->b1);
				wrefresh(win);
			}
		}
		else if (ch >= 32 && ch <= 126 && index1 < UBUF - 1) {
			for (int k = index1; k >= cursor_pos; k--) {
				m2->b1[k + 1] = m2->b1[k];
			}
			m2->b1[cursor_pos] = ch;
			index1++;
			cursor_pos++;
			wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE);
			wclrtoeol(win);
			mvwprintw(win, m2->sub_m2.rows - 1, m2->sub_m2.x + SPACE, "%s", m2->b1);
			wrefresh(win);
		}
		else if (ch == ESC) {
			wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
			wclrtoeol(win);
			wrefresh(win);
			break;
		}
	}

	wmove(win, m2->sub_m2.rows - 1, m2->sub_m2.x);
	wclrtoeol(win);
	wrefresh(win);

	int i = 0;
	int index = match_cmd(m2->b1);
	int rows = 0, cols = 0;

	while (m2->b1[i] != ' ' && m2->b1[i] != '\0') i++;

	if (index != 8 && index != 1 && index != 10) { 
		rows = parse_int(m2->b1, &i);
		cols = parse_int(m2->b1, &i);
	}

	switch (index) {
		case 0:
			handle_goto_cmd(win, m, m2, c1, *head, rows, cols);
			break;
		case 1:
			handle_exit_cmd(win, f);
			break;
		case 2:
			handle_resize_cmd(win, m, m2, c1, head, rows, cols);
			break;
		case 3:
			handle_color_cmd(win, m, m2, c1, head, rows, cols, &i);
			break;
		case 4:
		case 5:
			handle_copy_move_cmd(win, m, m2, head, rows, cols, index, &i);
			break;
		case 6:
			handle_delete_cmd(win, m, m2, head, rows, cols, &i);
			break;
		case 8:
			handle_template_cmd(win, m2, head, f);
			break;
		case 9:
			handle_use_template_cmd(win, m2, m, head, c1, rows, cols, &i);
			break;
		case 10:
			handle_save_cmd(win, m, m2, f, head);
			break;
		default:
			err_msg(win, m2, 2);
			break;
	}
}
