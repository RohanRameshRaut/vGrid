#ifndef _initialise_H_
#define _initialise_H_
#include "types.h"

int validate_argv(int argc, char **argv, unsigned int **arr);
int get_digits(int n);
int initialise_argv(Matrix m[4], A_Matrix *m2, Cursor *c1, unsigned int *arr);

#endif
