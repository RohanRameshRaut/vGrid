#include "types.h"
#include <stdio.h>
#include <stdlib.h>

int read_file(FILE *f, uint32_t start_offset, Tcolor **head){
    uint32_t offset = start_offset;
    Tcolor *tail = NULL;

    if (fseek(f, 0, SEEK_END) != 0) return 0;
    uint32_t file_size = ftell(f);

    uint32_t end_boundary = (file_meta.template_offset == INVALID_OFFSET || file_meta.template_offset > file_size) 
                            ? file_size 
                            : file_meta.template_offset;

    if (offset >= end_boundary) return 1; 

    while(file_meta.node_count != 0){
        Tcolor_file rec;
        if (fseek(f, offset, SEEK_SET) != 0) return 0;

        if(fread(&rec, sizeof(rec), 1, f) != 1) {
            break;
        }

        Tcolor *node = malloc(sizeof(Tcolor));
        if (!node) return 0;

        node->x = rec.x;
        node->y = rec.y;
        node->id = rec.id;
        node->next = NULL;

        if(*head == NULL){
            *head = node;
        }
        else{
            tail->next = node;
        }

        tail = node;
        offset = ftell(f);
	file_meta.node_count -= 1;
    }
    return 1;
}
