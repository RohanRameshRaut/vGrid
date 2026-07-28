#ifndef __TEMPLATE_PARSER_H_
#define __TEMPLATE_PARSER_H_
#include <ncurses.h>
#include <stdlib.h>
#include "types.h"

int save_template(Template *t, FILE *f);
void search_template_xy(Template *t);
uint32_t search_template(const unsigned char *t);
int search_cell(int cell[], Tcolor_file *cell_rec, Tcolor **head);
int validate_template_parser_function(Template *t, Template_parser template_parser, Tcolor **head);
void template_parser_function(char arr[UBUF], Template_parser *template_parser);

#endif
