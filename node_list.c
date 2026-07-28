#include "types.h"
#include "node_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <limits.h>

uint8_t hex_to_rgb(const color *arr, const char *hex){
	unsigned int r_255 = 0;
	unsigned int g_255 = 0;
	unsigned int b_255 = 0;

	if (hex == NULL) return 0;

	if (*hex == '#') hex++;

	if (sscanf(hex, "%02x%02x%02x", &r_255, &g_255, &b_255) != 3) return 0;

	uint8_t bestMatch = 0;
	int bestDistance = INT_MAX;

	for (int i = 0; i < 256; i++) {
		int dr = (int)r_255 - (int)arr[i].r;
		int dg = (int)g_255 - (int)arr[i].g;
		int db = (int)b_255 - (int)arr[i].b;

		int distance = dr * dr + dg * dg + db * db;

		if (distance < bestDistance) {
			bestDistance = distance;
			bestMatch = (uint8_t)i;
		}
	}

	return bestMatch;
}

void color_palette(color *color_arr){

	for (int i = 0; i < 216; i++) {
		int r = (i / 36) % 6;
		int g = (i / 6) % 6;
		int b = i % 6;

		color_arr[i + 16].r = r ? r * 40 + 55 : 0;
		color_arr[i + 16].g = g ? g * 40 + 55 : 0;
		color_arr[i + 16].b = b ? b * 40 + 55 : 0;
	}

	for (int i = 0; i < 24; i++) {
		int v = 8 + i * 10;

		color_arr[i + 232].r = v;
		color_arr[i + 232].g = v;
		color_arr[i + 232].b = v;
	}

	for (int c = 16; c < 256; c++){
		short r_1000 = (short)((color_arr[c].r * 1000) / 255);
		short g_1000 = (short)((color_arr[c].g * 1000) / 255);
		short b_1000 = (short)((color_arr[c].b * 1000) / 255);
		init_color(c, r_1000, g_1000, b_1000);
		init_pair(c, c, c);
	}
}

void link_tcolor(Tcolor **head, uint8_t color_id, int x, int y, unsigned char overlap) {
	Tcolor *new_node = malloc(sizeof(Tcolor));
	if (!new_node) return;
	new_node->id = color_id;
	new_node->x = x;
	new_node->y = y;
	new_node->next = NULL;

	if (*head == NULL) {
		*head = new_node;
		return;
	}

	Tcolor *curr = *head;
	Tcolor *prev = NULL;

	while (curr) {
		if (curr->y > y) break;
		if (curr->y == y) {
			if (curr->x > x) break;

			if (curr->x == x) {
				if (overlap == 1) {
				} else {
					break;
				}
			}
		}
		prev = curr;
		curr = curr->next;
	}

	if (prev == NULL) {
		new_node->next = *head;
		*head = new_node;
	} else {
		new_node->next = curr;
		prev->next = new_node;
	}
}

void merge_list(Tcolor **head, Tcolor **head1){
	if (!head || !head1 || !*head1) return;

	Tcolor *node = *head1;
	if (*head == NULL) {
		*head = node;
		return;
	}

	Tcolor *curr = *head;
	Tcolor *prev = NULL;
	while (curr) {
		if (curr->y > node->y) break;
		if (curr->y == node->y && curr->x > node->x) break;
		prev = curr;
		curr = curr->next;
	}
	if (!prev) {
		node->next = *head;
		*head = node;
	} else {
		node->next = curr;
		prev->next = node;
	}
}
