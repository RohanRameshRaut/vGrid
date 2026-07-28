#ifndef _READ_FILE_H_
#define _READ_FILE_H_
#include "types.h"
#include<stdio.h>

int read_file(FILE *f, uint32_t start_offset, Tcolor **head);

#endif
