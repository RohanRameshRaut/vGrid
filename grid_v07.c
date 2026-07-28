#include <ncurses.h>
#include <stdlib.h>
#include "_initialise.h"
#include "types.h"
#include "matrix03.h"
#include "navigation.h"
#include "node_list.h"
#include "colon_mode.h"
#include "write_file.h"
#include "read_file.h"
#include <unistd.h>
#include <sys/wait.h>

ChunkInfo chunk;
Metadata file_meta;
color color_arr[UBUF];
FILE *ff = NULL;

WINDOW *create_win(A_Matrix m2, int starty, int startx) {
	WINDOW *local_win = newwin(m2.sub_m2.rows, m2.sub_m2.cols, starty, startx);
	wrefresh(local_win);
	return local_win;
}

int main(int argc, char **argv) {
	if (argc < FORMATED || argc > USER) { printf("argc err !!\n"); return 2; }
	if(argv[1][0] != '1' && argv[1][0] != '0') { printf("File type required!!\n"); return 2; }

	chunk.prev_offset = INVALID_OFFSET;
	file_meta.head_offset = sizeof(Metadata);
	chunk.first_offset = file_meta.head_offset;
	Matrix m[4];
	Cursor c1;
	A_Matrix m2;
	Tcolor *head = NULL;
	FILE *f;
	pid_t pid;

	initscr();
	if (has_colors() == FALSE || can_change_color() == FALSE) {
		endwin();
		printf("Your terminal does not support custom colors.\n");
		return 1;
	}
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_GREEN);
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	getmaxyx(stdscr, m2.sub_m2.rows, m2.sub_m2.cols);

	if (argc == FORMATED && argv[1][0] == '1'){ 
		f = fopen(argv[2], "rb+");
		if(!f) {
			endwin();
			fprintf(stderr, "Error: Could not open file %s\n", argv[2]);
			return 1;
		}
		if(fread(&file_meta, sizeof(Metadata), 1, f) != 1) {
			endwin();
			fclose(f);
			fprintf(stderr, "Error: File structure is empty or corrupted.\n");
			return 1;
		}
		pid = fork();

		if(pid == 0){//child
			execlp("cp", "cp", argv[2], "null.ite", (char *) NULL);//Your program is replaced by cp, so the next line is never executed.
			perror("execlp error");
			return 1;
		} else{//parent
			wait(NULL);
			ff = fopen("null.ite", "rb+");
			if(!ff){
				perror("fopen error");
				return 1;
			}
		}

		m[0].rows = file_meta.cell_row;
		m[0].cols = file_meta.cell_col;
		m[1].rows = file_meta.grid_row;
		m[1].cols = file_meta.grid_col;
		unsigned int arr[] = {m[0].rows, m[0].cols, m[1].rows, m[1].cols};

		if(!initialise_argv(m, &m2, &c1, arr)){
			endwin();
			fclose(f);
			fprintf(stderr, "Incorrect grid info!\n");
			exit(EXIT_FAILURE);
		}

		//	int k = (m[2].cols - B_PX) / (m[0].cols + B_PX);
		//	int l = (m[2].rows - B_PX) / (m[0].rows + B_PX);
		//int count = (k*l);
	//	int count = (m[3].rows * m[3].cols); //read all the cells
		read_file(f, file_meta.head_offset, &head);
	}
	else if(argc == USER && argv[1][0] == '0'){ 
		f = fopen(argv[6], "rb+");
		file_meta.head_offset = INVALID_OFFSET;
		file_meta.template_offset = INVALID_OFFSET;
		if(!f) {
			endwin();
			fprintf(stderr, "Error: Could not open user file %s\n", argv[6]);
			return 1;
		}
		pid = fork();

		if(pid == 0){//child
			execlp("cp", "cp", argv[6], "null.ite", (char *) NULL);//Your program is replaced by cp, so the next line is never executed.
			perror("execlp error");
			return 1;
		} else{//parent
			wait(NULL);
			ff = fopen("null.ite", "rb+");
			if(!ff){
				perror("fopen error");
				return 1;
			}
		}
		unsigned int *arr = NULL;
		if(!validate_argv(argc, argv, &arr)){
			endwin();
			fclose(f);
			fprintf(stderr, "arguments are not valid!\n");
			exit(EXIT_FAILURE);
		}
		if(!initialise_argv(m, &m2, &c1, arr)){
			endwin();
			fclose(f);
			fprintf(stderr, "dimensions exceed limits!\n");
			free(arr);
			exit(EXIT_FAILURE);
		}
		free(arr);
	}
	else { 
		endwin();
		printf("argc err\n"); 
		return 2; 
	}

	for (int i = 0; i < UBUF; i++) {
		m2.b2[i] = malloc(UBUF * sizeof(char));
		if (m2.b2[i] == NULL) {
			endwin();
			fclose(f);
			fprintf(stderr, "Out of memory for history data buffers!\n");
			return 1;
		}
	}

	WINDOW *win = create_win(m2, 0, 0);
	cell(win, m);
	set_number(win, m2, m);
	color_palette(color_arr);
	update_grid_content(win, m, head);
	wmove(win, c1.pcy, c1.pcx);
	keypad(win, TRUE);
	wrefresh(win);

	int running = 1, ch = 0;
	int input_num = 0, digit_flag = 0;
	int jump = 0;

	while (running) {
		ch = wgetch(win);

		if (IS_LEFT(ch) || IS_RIGHT(ch) || IS_UP(ch) || IS_DOWN(ch)){
			jump = digit_flag ? input_num : 1;

			if (IS_LEFT(ch)) { go_left(win, m, m2, &c1, jump, head); }
			else if (IS_RIGHT(ch)) { go_right(win, m, m2, &c1, jump, head); }
			else if (IS_UP(ch)) { go_up(win, m, m2, &c1, jump, head); }
			else { go_down(win, m, m2, &c1, jump, head); }

			digit_flag = 0;
			input_num = 0;
		}
		else if (ch == COLON){
			colon_mode(win, m, &m2, &c1, &head, f);
			wmove(win, c1.pcy, c1.pcx);
		}
		else if ('0' <= ch && ch <='9'){
			input_num = (input_num*10) + (ch - '0');
			digit_flag = 1;
		}
		m[3].x = c1.ccx/m[0].cols;
		m[3].y = c1.ccy/m[0].rows;

		if (m2.sub_m2.cols > 20 && m2.sub_m2.rows > 0) {
			mvwprintw(win, m2.sub_m2.rows-1, m2.sub_m2.cols-20, "                ");
			mvwprintw(win, m2.sub_m2.rows-1, m2.sub_m2.cols-15, "%d, %d", m[3].y, m[3].x);
			mvwprintw(win, m2.sub_m2.rows-1, m2.sub_m2.cols-20, "%c", ch);
		}
		wmove(win, c1.pcy, c1.pcx);
		wrefresh(win);
	}
	pid = fork();

	if(pid == 0){//child
		execlp("rm", "rm", "null.ite", (char *) NULL);
		perror("execlp error");
		return 1;
	} else{//let the child finish, otherwise the parent may continue before the child finishes.
		wait(NULL);
	}

	for (int i = 0; i < UBUF; i++) {
		free(m2.b2[i]);
	}
	free(m2.T_index);
	free(m2.L_index);
	free(m2.R_index);
	fclose(f);
	delwin(win);
	endwin();
	return 0;
}
