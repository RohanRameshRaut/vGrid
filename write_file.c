#include "write_file.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

int write_in_file(FILE *f, Matrix m[4], Tcolor **head){
	file_meta.cell_row = m[0].rows;
	file_meta.cell_col = m[0].cols;
	file_meta.grid_row = m[3].rows;
	file_meta.grid_col = m[3].cols;

	file_meta.head_offset = sizeof(Metadata);
	file_meta.node_count = 0;

	if (fseek(f, sizeof(Metadata), SEEK_SET) != 0) return 0;

	Tcolor *curr = *head;
	while (curr){
		Tcolor_file rec;
		rec.x = curr->x;
		rec.y = curr->y;
		rec.id = curr->id;
		if (fwrite(&rec, sizeof(rec), 1, f) != 1) return 0;
		file_meta.node_count += 1;
		curr = curr->next;
	}

	if (ff != NULL && file_meta.template_offset != INVALID_OFFSET) {
		Template_file t;
		Template tt;
		uint32_t offset = file_meta.template_offset;

		file_meta.template_offset = ftell(f);

		while(offset != INVALID_OFFSET){
			if (fseek(ff, offset, SEEK_SET) != 0) return 0;
			if(fread(&t, sizeof(Template_file), 1, ff) != 1) return 0;

			if (t.next_offset == offset) {
				break;
			}
			tt.name = NULL;
			tt.c = NULL;
			tt.t = NULL;

			if (t.name_len > 0) {
				tt.name = malloc(t.name_len * sizeof(unsigned char));
				if(!tt.name) break;
				if(fread(tt.name, sizeof(unsigned char), t.name_len, ff) != (size_t)t.name_len) {
					free(tt.name);
					break;
				}
			}

			if (t.cell_count > 0) {
				tt.c = malloc(t.cell_count * sizeof(Tcolor_template));
				if(!tt.c) {
					free(tt.name);
					break;
				}
				if(fread(tt.c, sizeof(Tcolor_template), t.cell_count, ff) != (size_t)t.cell_count) {
					free(tt.name);
					free(tt.c);
					break;
				}
			}

			if (t.temp_count > 0) {
				tt.t = malloc(t.temp_count * sizeof(uint32_t));
				if(!tt.t) {
					free(tt.name);
					free(tt.c);
					break;
				}
				if(fread(tt.t, sizeof(uint32_t), t.temp_count, ff) != (size_t)t.temp_count) {
					free(tt.name);
					free(tt.c);
					free(tt.t);
					break;
				}
				//add the current files offset in all the array values 
				for(int i=0;i<t.temp_count;i++){
					tt.t[i] += file_meta.template_offset;
				}
			}

			uint32_t old_next_offset = t.next_offset;

			if (old_next_offset == INVALID_OFFSET) {
				t.next_offset = INVALID_OFFSET;
			} else {
				t.next_offset = ftell(f) + sizeof(Template_file) + t.name_len +
					(sizeof(Tcolor_template) * t.cell_count) +
					(sizeof(uint32_t) * t.temp_count);
			}

			// Write directly to destination file 'f'
			fwrite(&t, sizeof(Template_file), 1, f);
			if (t.name_len > 0) fwrite(tt.name, sizeof(unsigned char), t.name_len, f);
			if (t.cell_count > 0) fwrite(tt.c, sizeof(Tcolor_template), t.cell_count, f);
			if (t.temp_count > 0) fwrite(tt.t, sizeof(uint32_t), t.temp_count, f);

			free(tt.name);
			free(tt.c);
			free(tt.t);

			offset = old_next_offset;
		}
	} else {
		file_meta.template_offset = INVALID_OFFSET;
	}
	if (fseek(f, 0, SEEK_SET) != 0) return 0;
	size_t data = fwrite(&file_meta, sizeof(Metadata), 1, f);
	if(data != 1) return 0;

	return 1;
}

