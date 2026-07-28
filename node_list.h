#ifndef __NODE_LIST_H_
#define __NODE_LIST_H_
#include "types.h"

uint8_t hex_to_rgb(const color *arr, const char *hex);
void color_palette(color *color_arr);
void link_tcolor(Tcolor **head, uint8_t color_id, int x, int y, unsigned char overlap);
void merge_list(Tcolor **head, Tcolor **head1);

#endif
