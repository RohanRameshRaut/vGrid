#include "_initialise.h"
#include <stdlib.h>

int validate_argv(int argc, char **argv, unsigned int **arr){
        unsigned int n=0;
        *arr = malloc(PARAMETERS * (sizeof(unsigned int)));
        if (*arr == NULL) return 0;

        for(unsigned char i=2; i<argc-1; i++){
                unsigned int num = 0;
                for(unsigned char j=0; argv[i][j] != '\0'; j++){
                        if(!('0' <= argv[i][j] && argv[i][j] <= '9')){ 
                                return 0;
                        }
                        else if('0' <= argv[i][j] && argv[i][j] <= '9'){
                                num = (num * 10) + (argv[i][j] - '0');
                        }
                }
                (*arr)[n++] = num;
        }
        return 1;
}

int get_digits(int n) {
        int count = 0;
        if (n == 0) return 1;
        while (n != 0) { n = n / 10; ++count; }
        return count;
}

int initialise_argv(Matrix m[4], A_Matrix *m2, Cursor *c1, unsigned int *arr){
        if(arr[0] < 1 || arr[1] < 1 || arr[2] < 1 || arr[3] < 1 ){ return 0; }
        if(arr[0] > (unsigned int)m2->sub_m2.rows || arr[1] > (unsigned int)m2->sub_m2.cols){ return 0; }
        m[0].x = 0, m[0].y = 0;
        m[0].rows = arr[0], m[0].cols = arr[1];
        m[1].x = 0, m[1].y = 0;
        m[1].rows = (m[0].rows * arr[2]), m[1].cols = (m[0].cols * arr[3]);
        m2->sub_m2.x = 0, m2->sub_m2.y = 0;
        int start_x = get_digits(m[1].rows);
        m[2].rows = m2->sub_m2.rows - (T_MARGIN + B_MARGIN);
        m[2].cols = m2->sub_m2.cols - start_x - (R_MARGIN);
        m[2].x = m2->sub_m2.x + start_x;
        m[2].y = m2->sub_m2.y + (T_MARGIN);

        int k = (m[2].cols - B_PX) / (m[0].cols + B_PX);
        int l = (m[2].rows - B_PX) / (m[0].rows + B_PX);
        m[3].rows = arr[2];
        m[3].cols = arr[3];

        int cdk = MINIMUM(m[3].cols, k);
        int rdk = MINIMUM(m[3].rows, l);

        m[2].cols = (((m[0].cols + B_PX) * cdk) + B_PX);
        m[2].rows = (((m[0].rows + B_PX) * rdk) + B_PX);
        m2->T_index = malloc((k + 2) * sizeof(unsigned char));
        m2->L_index = malloc((m[2].rows + 1) * sizeof(int));
        m2->R_index = malloc((m[2].rows + 1) * sizeof(int));

        c1->pcx = m[2].x + B_PX;
        c1->pcy = m[2].y + B_PX;
        c1->ccx = 0;
        c1->ccy = 0;

        if(m[0].rows > m2->sub_m2.rows || m[0].cols > m2->sub_m2.cols){
                return 0;
        }

        return 1;
}
