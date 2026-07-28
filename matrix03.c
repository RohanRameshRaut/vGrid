#include "matrix03.h"

void cell(WINDOW *local_win, Matrix m[3]){
	for (int i = 0; i < m[2].rows; i++) {
		if (i % (m[0].rows + B_PX) == 0) {
			for (int j = 0; j < m[2].cols; j++) {
				if (j % (m[0].cols + B_PX) == 0) mvwprintw(local_win, i + m[2].y, j + m[2].x, "%c", C_CHAR);
				else mvwprintw(local_win, i + m[2].y, j + m[2].x, "%c", H_CHAR);
			}
		} else {
			for (int j = 0; j < m[2].cols; j++) {
				if (j % (m[0].cols + B_PX) == 0) mvwprintw(local_win, i + m[2].y, j + m[2].x, "%c", V_CHAR);
			}
		}
	}
	wrefresh(local_win);
}

void update_grid_content(WINDOW *local_win, Matrix m[3], Tcolor *head){
	for (int i = 0; i < m[2].rows; i++) {
		if (i % (m[0].rows + B_PX) == 0) continue;

		int cell_y_index = i / (m[0].rows + B_PX);
		int inside_y = (i % (m[0].rows + B_PX)) - B_PX;
		int data_row = m[1].y + (cell_y_index * m[0].rows) + inside_y;

		for (int j = 0; j < m[2].cols; j++) {
			if (j % (m[0].cols + B_PX) == 0) continue;

			int cell_x_index = j / (m[0].cols + B_PX);
			int inside_x = (j % (m[0].cols + B_PX)) - B_PX;
			int data_col = m[1].x + (cell_x_index * m[0].cols) + inside_x;

			if (data_row < m[1].rows && data_col < m[1].cols && data_row >= 0 && data_col >= 0) {
				Tcolor *temp = head;
				Tcolor *last_matched_node = NULL;

				while (temp) {
					if (data_row >= temp->y * m[0].rows && data_row < temp->y * m[0].rows + m[0].rows && data_col >= temp->x * m[0].cols && data_col < temp->x * m[0].cols + m[0].cols) {
						last_matched_node = temp;
					}
					temp = temp->next;
				}

				if (last_matched_node) {
					wattron(local_win, COLOR_PAIR(last_matched_node->id) | A_BOLD);
					mvwaddch(local_win, i + m[2].y, j + m[2].x, M_CHAR);
					wattroff(local_win, COLOR_PAIR(last_matched_node->id) | A_BOLD);
				} else {
					mvwaddch(local_win, i + m[2].y, j + m[2].x, M_CHAR);
				}
			}
		}
	}
	wrefresh(local_win);
}

void set_number(WINDOW *local_win, A_Matrix m2, Matrix m[3]){
	int visible_cells_y = (m[2].rows - B_PX) / (m[0].rows + B_PX);
	int visible_cells_x = (m[2].cols - B_PX) / (m[0].cols + B_PX);
	for (int i = 0; i < m2.sub_m2.rows; i++) {
		for (int j = 0; j < m[2].x; j++) mvwprintw(local_win, i, j, " ");
	}
	for (int j = m[2].x; j < m2.sub_m2.cols; j++) mvwprintw(local_win, 0, j, " ");

	for (int i = 0; i < visible_cells_y; i++) m2.L_index[i] = (m[1].y / m[0].rows) + i;
	for (int i = 0, j = 0; j < visible_cells_y; i += m[0].rows + B_PX, j++) {
		mvwprintw(local_win, m[2].y + i + (m[0].rows / 2), m2.sub_m2.x, "%d", m2.L_index[j]);
	}
	for (int i = 0; i < visible_cells_x; i++) m2.T_index[i] = ((m[1].x / m[0].cols) + i) % 10;
	for (int i = 0, j = 0; j < visible_cells_x; i += m[0].cols + B_PX, j++) {
		mvwprintw(local_win, 0, m[2].x + i + ((m[0].cols / 2) + B_PX), "%d", m2.T_index[j]);
	}
	wrefresh(local_win);
}
