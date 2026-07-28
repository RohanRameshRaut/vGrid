#include "template_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PAIR 2
#define RANGE 4

static int custom_strcmp(const char *s1, const char *s2) {
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static int sync_files(FILE *f) {
	if (ff != NULL) {
		fflush(ff);
	}
	if (f != NULL) {
		if (fseek(f, 0, SEEK_SET) != 0) return 0;
		if (fwrite(&file_meta, sizeof(Metadata), 1, f) != 1) return 0;
		fflush(f);
		return 1;
	}
	return 0;
}

int save_template(Template *t, FILE *f) {
	if (ff == NULL || f == NULL || t == NULL) return 0;

	Template_file new_template_rec, template_rec, prev_template_rec;
	uint32_t offset = file_meta.template_offset;
	uint32_t prev_offset = INVALID_OFFSET, new_offset;

	if (fseek(ff, 0, SEEK_END) != 0) return 0;
	new_offset = ftell(ff);

	if (offset != INVALID_OFFSET && offset >= new_offset) {
		offset = INVALID_OFFSET;
		file_meta.template_offset = INVALID_OFFSET;
	}

	new_template_rec.min_x = t->min_x;
	new_template_rec.min_y = t->min_y;
	new_template_rec.max_x = t->max_x;
	new_template_rec.max_y = t->max_y;
	new_template_rec.name_len = t->name_len;
	new_template_rec.cell_count = t->cell_count;
	new_template_rec.temp_count = t->temp_count;

	new_template_rec.n = sizeof(Template_file);
	new_template_rec.c = new_template_rec.n + (sizeof(unsigned char) * new_template_rec.name_len);
	new_template_rec.t = new_template_rec.c + (sizeof(Tcolor_template) * new_template_rec.cell_count);
	new_template_rec.next_offset = INVALID_OFFSET;

	// Case 1: First template in file
	if (offset == INVALID_OFFSET) {
		fseek(ff, new_offset, SEEK_SET);
		fwrite(&new_template_rec, sizeof(Template_file), 1, ff);
		if (t->name && t->name_len > 0) fwrite(t->name, sizeof(unsigned char), t->name_len, ff);
		if (t->c && t->cell_count > 0) fwrite(t->c, sizeof(Tcolor_template), t->cell_count, ff);
		if (t->t && t->temp_count > 0) fwrite(t->t, sizeof(uint32_t), t->temp_count, ff);

		t->next_offset = INVALID_OFFSET;
		file_meta.template_offset = new_offset;

		return sync_files(f);
	}

	// Case 2: Traverse linked list via ff to find insertion spot
	int chain_corrupted = 0;

	while (offset != INVALID_OFFSET) {
		if (offset >= (uint32_t)new_offset) { chain_corrupted = 1; break; }

		if (fseek(ff, offset, SEEK_SET) != 0) { chain_corrupted = 1; break; }
		if (fread(&template_rec, sizeof(Template_file), 1, ff) != 1) { chain_corrupted = 1; break; }

		if (template_rec.n != sizeof(Template_file) ||
				(uint32_t)offset + template_rec.n + template_rec.name_len > (uint32_t)new_offset) {
			chain_corrupted = 1;
			break;
		}

		unsigned char *name = malloc((template_rec.name_len + 1) * sizeof(unsigned char));
		if (!name) break;

		if (fread(name, sizeof(unsigned char), template_rec.name_len, ff) != (size_t)template_rec.name_len) {
			free(name);
			chain_corrupted = 1;
			break;
		}
		name[template_rec.name_len] = '\0';

		int i = 0, t_is_smaller = 0;
		if (t->name) {
			while (t->name[i] != '\0' && name[i] != '\0') {
				if (t->name[i] < name[i]) { t_is_smaller = 1; break; }
				if (t->name[i] > name[i]) { t_is_smaller = 0; break; }
				i++;
			}
			if (t->name[i] == name[i]) {
				t_is_smaller = 0;
			} else if (t->name[i] == '\0') {
				t_is_smaller = 1;
			} else if (name[i] == '\0') {
				t_is_smaller = 0;
			}
		}

		free(name);
		if (t_is_smaller) break;

		prev_offset = offset;

		if (template_rec.next_offset == offset) { chain_corrupted = 1; break; }
		offset = template_rec.next_offset;
	}

	if (chain_corrupted) {
		t->next_offset = INVALID_OFFSET;
		new_template_rec.next_offset = INVALID_OFFSET;

		fseek(ff, new_offset, SEEK_SET);
		fwrite(&new_template_rec, sizeof(Template_file), 1, ff);
		if (t->name && t->name_len > 0) fwrite(t->name, sizeof(unsigned char), t->name_len, ff);
		if (t->c && t->cell_count > 0) fwrite(t->c, sizeof(Tcolor_template), t->cell_count, ff);
		if (t->t && t->temp_count > 0) fwrite(t->t, sizeof(uint32_t), t->temp_count, ff);

		file_meta.template_offset = new_offset;
		return sync_files(f);
	}

	if (prev_offset == INVALID_OFFSET) {
		t->next_offset = file_meta.template_offset;
		new_template_rec.next_offset = t->next_offset;

		fseek(ff, new_offset, SEEK_SET);
		fwrite(&new_template_rec, sizeof(Template_file), 1, ff);
		if (t->name && t->name_len > 0) fwrite(t->name, sizeof(unsigned char), t->name_len, ff);
		if (t->c && t->cell_count > 0) fwrite(t->c, sizeof(Tcolor_template), t->cell_count, ff);
		if (t->t && t->temp_count > 0) fwrite(t->t, sizeof(uint32_t), t->temp_count, ff);

		file_meta.template_offset = new_offset;
		return sync_files(f);
	} else {
		t->next_offset = offset;
		new_template_rec.next_offset = t->next_offset;

		fseek(ff, new_offset, SEEK_SET);
		fwrite(&new_template_rec, sizeof(Template_file), 1, ff);
		if (t->name && t->name_len > 0) fwrite(t->name, sizeof(unsigned char), t->name_len, ff);
		if (t->c && t->cell_count > 0) fwrite(t->c, sizeof(Tcolor_template), t->cell_count, ff);
		if (t->t && t->temp_count > 0) fwrite(t->t, sizeof(uint32_t), t->temp_count, ff);

		fseek(ff, prev_offset, SEEK_SET);
		if (fread(&prev_template_rec, sizeof(Template_file), 1, ff) == 1) {
			prev_template_rec.next_offset = new_offset;
			fseek(ff, prev_offset, SEEK_SET);
			fwrite(&prev_template_rec, sizeof(Template_file), 1, ff);
		}

		return sync_files(f);
	}
}

void search_template_xy(Template *t) {
	if (!t || ff == NULL) return;

	struct IntPair {
		int min_x, min_y;
		int max_x, max_y;
	};

	struct IntPair result;
	result.min_x = (int)INVALID_OFFSET;
	result.min_y = (int)INVALID_OFFSET;
	result.max_x = -1;
	result.max_y = -1;

	if (t->t != NULL && t->temp_count > 0) {
		Template_file template_rec;
		for (int i = 0; i < t->temp_count; i++) {
			fseek(ff, t->t[i], SEEK_SET);
			if (fread(&template_rec, sizeof(Template_file), 1, ff) != 1) break;

			if (result.min_x == (int)INVALID_OFFSET || template_rec.min_x < result.min_x) result.min_x = template_rec.min_x;    
			if (template_rec.max_x > result.max_x) result.max_x = template_rec.max_x;    
			if (result.min_y == (int)INVALID_OFFSET || template_rec.min_y < result.min_y) result.min_y = template_rec.min_y;    
			if (template_rec.max_y > result.max_y) result.max_y = template_rec.max_y;    
		}
	}

	if (t->c && t->cell_count > 0) {
		int max_x = -1, max_y = -1;
		int min_x = (int)INVALID_OFFSET;
		int min_y = (int)INVALID_OFFSET;

		for (int i = 0; i < t->cell_count; i++) {
			if (t->c[i].x > max_x) max_x = t->c[i].x;
			if (t->c[i].y > max_y) max_y = t->c[i].y;
			if (min_x == (int)INVALID_OFFSET || t->c[i].x < min_x) min_x = t->c[i].x;
			if (min_y == (int)INVALID_OFFSET || t->c[i].y < min_y) min_y = t->c[i].y;
		}

		if (result.min_x == (int)INVALID_OFFSET) {
			result.min_x = min_x;
			result.min_y = min_y;
			result.max_x = max_x;
			result.max_y = max_y;
		} else {
			if (max_x > result.max_x) result.max_x = max_x;
			if (max_y > result.max_y) result.max_y = max_y;
			if (min_y < result.min_y) result.min_y = min_y;
			if (min_x < result.min_x) result.min_x = min_x;
		}
	}

	t->min_x = result.min_x;
	t->min_y = result.min_y;
	t->max_x = result.max_x;
	t->max_y = result.max_y;
}

uint32_t search_template(const unsigned char *t) {
	if (!t || ff == NULL || file_meta.template_offset == INVALID_OFFSET) return INVALID_OFFSET;

	if (fseek(ff, 0, SEEK_END) != 0) return INVALID_OFFSET;
	long ff_size = ftell(ff);
	if (ff_size < 0) return INVALID_OFFSET;

	if (file_meta.template_offset >= (uint32_t)ff_size) {
		file_meta.template_offset = INVALID_OFFSET;
		return INVALID_OFFSET;
	}

	Template_file template_rec;
	uint32_t offset = file_meta.template_offset;

	while (offset != INVALID_OFFSET) {
		if (offset >= (uint32_t)ff_size) break;

		if (fseek(ff, offset, SEEK_SET) != 0) break;
		if (fread(&template_rec, sizeof(Template_file), 1, ff) != 1) break;

		if (template_rec.next_offset == offset) break;

		if (template_rec.n != sizeof(Template_file) ||
				offset + template_rec.n + template_rec.name_len > (uint32_t)ff_size) {
			break;
		}

		if (template_rec.name_len == 0) {
			offset = template_rec.next_offset;
			continue;
		}

		unsigned char *name = malloc((template_rec.name_len + 1) * sizeof(unsigned char));
		if (!name) break;

		if (fseek(ff, offset + template_rec.n, SEEK_SET) != 0) {
			free(name);
			break;
		}

		if (fread(name, sizeof(unsigned char), template_rec.name_len, ff) != (size_t)template_rec.name_len) {
			free(name);
			break;
		}
		name[template_rec.name_len] = '\0';

		if (custom_strcmp((const char *)t, (const char *)name) == 0) {
			free(name);
			return offset; // Found matching template!
		}

		free(name);
		offset = template_rec.next_offset;
	}
	return INVALID_OFFSET;
}

int search_cell(int cell[], Tcolor_file *cell_rec, Tcolor **head) {
	if (!head || !*head || !cell_rec) return 0;
	Tcolor *temp = *head;
	int found = 0;

	while (temp != NULL) {
		if (temp->y == cell[0] && temp->x == cell[1]) {
			cell_rec->y = temp->y;
			cell_rec->x = temp->x;
			cell_rec->id = temp->id;
			found = 1;
			break; 
		}
		temp = temp->next;
	}
	return found;
}

int validate_template_parser_function(Template *t, Template_parser template_parser, Tcolor **head) {
	if (!t || !template_parser.template_name) return 1;
	uint32_t temp_late = search_template(template_parser.template_name);
	Template *new_template = NULL;

	if (temp_late != INVALID_OFFSET) {
		return 2; // Template already exists
	}

	new_template = calloc(1, sizeof(Template));
	if (!new_template) return 1;

	new_template->name = (char *)template_parser.template_name;
	new_template->name_len = template_parser.template_name_len;
	new_template->temp_count = template_parser.template_cnt;
	new_template->t = NULL;
	new_template->c = NULL;

	if (template_parser.template_cnt > 0) {
		new_template->t = malloc(template_parser.template_cnt * sizeof(uint32_t));
		if (!new_template->t) {
			free(new_template);
			return 1;
		}

		for (int i = 0; i < template_parser.template_cnt; i++) {
			uint32_t sub_offset = search_template(template_parser.template_arr[i]);
			if (sub_offset == INVALID_OFFSET) {
				free(new_template->t);
				free(new_template);
				return 3; // Sub-template does not exist
			}
			new_template->t[i] = sub_offset;
		}
	}

	int total_cells = template_parser.single_arr_cnt;
	for (int i = 0; i < template_parser.range_arr_cnt; i++) {
		int r1 = template_parser.range_arr[i][0], c1 = template_parser.range_arr[i][1];
		int r2 = template_parser.range_arr[i][2], c2 = template_parser.range_arr[i][3];
		int min_r = r1 < r2 ? r1 : r2, max_r = r1 > r2 ? r1 : r2;
		int min_c = c1 < c2 ? c1 : c2, max_c = c1 > c2 ? c1 : c2;
		total_cells += (max_r - min_r + 1) * (max_c - min_c + 1);
	}

	new_template->cell_count = total_cells;
	if (total_cells > 0) {
		new_template->c = malloc(total_cells * sizeof(Tcolor_template));
		if (!new_template->c) {
			if (new_template->t) free(new_template->t);
			free(new_template);
			return 1;
		}

		int current_idx = 0;
		Tcolor_file temp_cell;

		// Single cells
		for (int i = 0; i < template_parser.single_arr_cnt; i++) {
			if (!search_cell(template_parser.single_arr[i], &temp_cell, head)) {
				free(new_template->c);
				if (new_template->t) free(new_template->t);
				free(new_template);
				return 4; // Empty source cell
			}
			new_template->c[current_idx].x = temp_cell.x;
			new_template->c[current_idx].y = temp_cell.y;
			new_template->c[current_idx].id = temp_cell.id;
			current_idx++;
		}

		// Ranges
		for (int i = 0; i < template_parser.range_arr_cnt; i++) {
			int r1 = template_parser.range_arr[i][0], c1 = template_parser.range_arr[i][1];
			int r2 = template_parser.range_arr[i][2], c2 = template_parser.range_arr[i][3];
			int min_r = r1 < r2 ? r1 : r2, max_r = r1 > r2 ? r1 : r2;
			int min_c = c1 < c2 ? c1 : c2, max_c = c1 > c2 ? c1 : c2;

			for (int r = min_r; r <= max_r; r++) {
				for (int c = min_c; c <= max_c; c++) {
					int p[2] = { r, c };
					if (search_cell(p, &temp_cell, head)) {
						new_template->c[current_idx].x = temp_cell.x;
						new_template->c[current_idx].y = temp_cell.y;
						new_template->c[current_idx].id = temp_cell.id;
						current_idx++;
					}
				}
			}
		}
		new_template->cell_count = current_idx; // Adjust to actual existing nodes found
	}

	search_template_xy(new_template);
	*t = *new_template;
	free(new_template);
	return 0;
}


void template_parser_function(char arr[UBUF], Template_parser *template_parser) {
	int i = 0, maxlen = 0, colon_found = 0, range = 0, pair = 0;
	unsigned char j = 0;

	template_parser->template_cnt = 0;
	template_parser->single_arr_cnt = 0;
	template_parser->range_arr_cnt = 0;
	template_parser->template_name = NULL;
	template_parser->template_arr = NULL;
	template_parser->single_arr = NULL;
	template_parser->range_arr = NULL;

	// Skip leading spaces
	while (arr[i] == ' ') i++;

	// Skip "template" prefix
	if (arr[i] == 't' && arr[i+1] == 'e' && arr[i+2] == 'm' &&
			arr[i+3] == 'p' && arr[i+4] == 'l' && arr[i+5] == 'a' &&
			arr[i+6] == 't' && arr[i+7] == 'e' &&
			(arr[i+8] == ' ' || arr[i+8] == '\0'))
	{
		i += 8;
		while (arr[i] == ' ') i++;
	}

	// Read template name (before '=')
	int name_start = i;
	while (arr[i] != '\0' && arr[i] != '=') {
		if (('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) {
			int len = 0;
			while ((('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) && arr[i] != '\0') {
				len++;
				i++;
			}
			if (len > maxlen) maxlen = len;
		} else {
			i++;
		}
	}

	maxlen++;
	template_parser->template_name = calloc(maxlen, sizeof(char));
	if (!template_parser->template_name) return;

	i = name_start;
	j = 0;
	while (arr[i] != '\0' && arr[i] != '=') {
		if (('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) {
			while ((('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) && arr[i] != '\0') {
				if (j < maxlen - 1) {
					template_parser->template_name[j++] = arr[i];
				}
				i++;
			}
			template_parser->template_name[j] = '\0';
			break;
		} else {
			i++;
		}
	}

	template_parser->template_name_len = j;

	while (arr[i] != '\0' && arr[i] != '=') {
		i++;
	}
	if (arr[i] == '=') {
		i++;
	}
	int body_start_index = i;

	// Parse body AFTER '='
	i = body_start_index;
	maxlen = 0;
	while (arr[i] != '\0') {
		if (arr[i] == '<') {
			colon_found = 0;
			while (arr[i] != '\0' && arr[i] != '>') {
				if (arr[i] == ':') colon_found = 1;
				i++;
			}
			if (colon_found) range++;
			else pair++;
			if (arr[i] == '>') i++;
		}
		// Sub-template names are words OUTSIDE of '<...>' brackets and '{...}' braces
		else if (('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z')) {
			int len = 0;
			while ((('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) && arr[i] != '\0') {
				len++;
				i++;
			}
			template_parser->template_cnt++;
			if (len > maxlen) maxlen = len;
		} else {
			i++;
		}
	}
	maxlen++;

	if (template_parser->template_cnt > 0) {
		template_parser->template_arr = malloc(template_parser->template_cnt * sizeof(char *));
		for (int k = 0; k < template_parser->template_cnt; k++) {
			template_parser->template_arr[k] = calloc(maxlen, sizeof(char));
		}
	}
	if (pair > 0) {
		template_parser->single_arr = malloc(pair * sizeof(int *));
		template_parser->single_arr_cnt = pair;
		for (int k = 0; k < pair; k++) {
			template_parser->single_arr[k] = malloc(PAIR * sizeof(int));
		}
	}
	if (range > 0) {
		template_parser->range_arr = malloc(range * sizeof(int *));
		template_parser->range_arr_cnt = range;
		for (int k = 0; k < range; k++) {
			template_parser->range_arr[k] = malloc(RANGE * sizeof(int));
		}
	}

	int current_temp_cnt = 0;
	i = body_start_index;
	int s = 0, r = 0;

	while (arr[i] != '\0') {
		if (arr[i] == '<') {
			int num[4] = {0, 0, 0, 0};
			int count = 0;
			i++;
			while (arr[i] != '\0' && arr[i] != '>') {
				if ('0' <= arr[i] && arr[i] <= '9') {
					int n = 0;
					while ('0' <= arr[i] && arr[i] <= '9') {
						n = (n * 10) + (arr[i] - '0');
						i++;
					}
					if (count < 4) num[count++] = n;
				} else {
					i++;
				}
			}
			if (count == 2 && template_parser->single_arr && s < template_parser->single_arr_cnt && template_parser->single_arr[s]) {
				template_parser->single_arr[s][0] = num[0];
				template_parser->single_arr[s][1] = num[1];
				s++;
			} else if (count == 4 && template_parser->range_arr && r < template_parser->range_arr_cnt && template_parser->range_arr[r]) {
				template_parser->range_arr[r][0] = num[0];
				template_parser->range_arr[r][1] = num[1];
				template_parser->range_arr[r][2] = num[2];
				template_parser->range_arr[r][3] = num[3];
				r++;
			}
			if (arr[i] == '>') i++;
		} else if (('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z')) {
			int local_len = 0;
			while ((('a' <= arr[i] && arr[i] <= 'z') || ('A' <= arr[i] && arr[i] <= 'Z') || ('0' <= arr[i] && arr[i] <= '9')) && arr[i] != '\0') {
				if (template_parser->template_arr && current_temp_cnt < template_parser->template_cnt && template_parser->template_arr[current_temp_cnt]) {
					if (local_len < maxlen - 1) {
						template_parser->template_arr[current_temp_cnt][local_len++] = arr[i];
					}
				}
				i++;
			}
			if (template_parser->template_arr && current_temp_cnt < template_parser->template_cnt && template_parser->template_arr[current_temp_cnt]) {
				template_parser->template_arr[current_temp_cnt][local_len] = '\0';
				current_temp_cnt++;
			}
		} else {
			i++;
		}
	}
}

