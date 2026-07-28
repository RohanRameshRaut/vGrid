#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define INVALID_OFFSET UINT32_MAX
#define PARAMETERS 4
#define FORMATED 3
#define USER 7
#define TEMPLATE_SIZE 16
#define C_OFFSET 9 //system color id's 0-8
#define OBJ_PARAMETERS 6
#define UBUF 256
#define CMD {"goto", "q", "resize", "color", "copy", "move", "delete", "mmove", "template", "use_template", "w"}
#define CMD_LEN 11
#define ERR_MSG {"incorrect parameters", "incorrect coordinates", "wrong command", "invalid color", "no enough space", "empty src cell", "memory allocation err", "err writing in file", "Written in File!", "Template already exists", "Template does'nt exist", "Template created", "Failed to create template", "Template can't fit"}
#define ER_MSG_LEN 14
#define QUESTIONS {"override(0), overlap(1):", "front(1), back(0):"}
#define QUESTIONS_LEN 2
#define SPACE 1
#define H_CHAR '-'
#define V_CHAR '|'
#define C_CHAR '+'
#define M_CHAR ' '
#define B_PX 1
#define T_MARGIN 1
#define B_MARGIN 2
#define R_MARGIN 1
#define MINIMUM(a,b) ((a) < (b) ? (a):(b))
#define IS_LEFT(x) (x == 104 || x == KEY_LEFT)
#define IS_DOWN(x) (x == 106 || x == KEY_DOWN)
#define IS_UP(x) (x == 107 || x == KEY_UP)
#define IS_RIGHT(x) (x == 108 || x == KEY_RIGHT)
#define ESC 27
#define COLON 58

typedef struct Matrix {
	int rows, cols;
	int x, y;
} Matrix;

typedef struct Cursor{
	int pcx, pcy;
	int ccx, ccy;
} Cursor;

typedef struct A_Matrix {
	Matrix sub_m2;
	unsigned char *T_index;
	int *L_index;
	int *R_index;
	char b1[UBUF];
	char *b2[UBUF];
} A_Matrix;

typedef struct Tcolor {
	int x, y;
	uint8_t id;
	struct Tcolor *next;
} Tcolor;

typedef struct color{
	uint8_t r, g, b;
} color;

extern color color_arr[];

typedef struct Tcolor_file {//structure to store nodes in file. 19 bytes per cell
	int x, y;
	uint8_t id;
} Tcolor_file;

typedef struct ChunkInfo {
	uint32_t prev_offset;
	uint32_t first_offset;
	uint32_t last_offset;
	uint32_t next_offset;
	int loaded_count;
} ChunkInfo;

typedef struct Metadata {
	int cell_row, cell_col;
	int grid_row, grid_col;
	uint32_t head_offset;
	uint32_t template_offset;
	uint32_t node_count;
} Metadata;

extern ChunkInfo chunk;
extern Metadata file_meta;

typedef struct Tcolor_template{
	int x, y;
	uint8_t id;
} Tcolor_template;

typedef struct Template {
	int min_x, min_y;
	int max_x, max_y;
	char *name;//name of the template, or get the size from the user;
	unsigned char name_len;
	uint32_t *t;
	int temp_count;
	Tcolor_template *c;//cells array
	int cell_count;
	uint32_t next_offset;
} Template;

typedef struct Template_file {
	int min_x, min_y;
	int max_x, max_y;
	unsigned char name_len;
	int cell_count;
	int temp_count;
	uint32_t n;//starting location of name array in file = sizeof(Template_file);
	uint32_t c;//starting location of cell array in file = sizeof(Template_file) + (sizeof(unsigned char) * name_len);
	uint32_t t;//starting location of template array = = sizeof(Template_file) + (sizeof(unsigned char) * name_len) + (sizeof(Tcolor_template) * cell_count) ;
	uint32_t next_offset;
} Template_file;

typedef struct Template_parser{
	unsigned char *template_name;
	unsigned char template_name_len;
	unsigned char **template_arr;
	unsigned char template_arr_max_len;
	int template_cnt;
	int **single_arr;
	int single_arr_cnt;
	int **range_arr;
	int range_arr_cnt;
} Template_parser;

extern FILE *ff;

#endif
