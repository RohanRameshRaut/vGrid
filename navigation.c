#include "navigation.h"
#include "matrix03.h"

void go_left(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head){
        int max_left = c1->ccx / m[0].cols;
        if (jump > max_left) jump = max_left;
        c1->ccx -= jump * m[0].cols;
        c1->pcx -= jump * (m[0].cols + B_PX);
        while (c1->pcx < (m[2].x + B_PX)) {
                m[1].x -= m[0].cols;
                set_number(win, m2, m);
                c1->pcx += (m[0].cols + B_PX);
        }
        update_grid_content(win, m, head);
        wmove(win, c1->pcy, c1->pcx);
}

void go_right(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head){
        int max_ccx = m[1].cols - m[0].cols;
        if (c1->ccx + jump * m[0].cols > max_ccx) jump = (max_ccx - c1->ccx) / m[0].cols;
        c1->ccx += jump * m[0].cols;
        c1->pcx += jump * (m[0].cols + B_PX);
        while (c1->pcx + m[0].cols > m[2].x + m[2].cols) {
                m[1].x += m[0].cols;
                set_number(win, m2, m);
                c1->pcx -= (m[0].cols + B_PX);
        }
        update_grid_content(win, m, head);
        wmove(win, c1->pcy, c1->pcx);
}

void go_up(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head){
        int max_up = c1->ccy / m[0].rows;
        if (jump > max_up) jump = max_up;
        c1->ccy -= jump * m[0].rows;
        c1->pcy -= jump * (m[0].rows + B_PX);
        while (c1->pcy < (m[2].y + B_PX)) {
                m[1].y -= m[0].rows;
                set_number(win, m2, m);
                c1->pcy += (m[0].rows + B_PX);
        }
        update_grid_content(win, m, head);
        wmove(win, c1->pcy, c1->pcx);
}

void go_down(WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, int jump, Tcolor *head){
        int max_ccy = m[1].rows - m[0].rows;
        if (c1->ccy + jump * m[0].rows > max_ccy) jump = (max_ccy - c1->ccy) / m[0].rows;
        c1->ccy += jump * m[0].rows;
        c1->pcy += jump * (m[0].rows + B_PX);
        while (c1->pcy + m[0].rows > m[2].y + m[2].rows) {
                m[1].y += m[0].rows;
                set_number(win, m2, m);
                c1->pcy -= (m[0].rows + B_PX);
        }
        update_grid_content(win, m, head);
        wmove(win, c1->pcy, c1->pcx);
}

void go_to(int rows, int cols, WINDOW *win, Matrix m[3], A_Matrix m2, Cursor *c1, Tcolor *head){
        if(rows < (c1->ccy/m[0].rows)) go_up(win, m, m2, c1, (c1->ccy/m[0].rows)-rows, head);
        if(rows > (c1->ccy/m[0].rows)) go_down(win, m, m2, c1, rows-(c1->ccy/m[0].rows), head);
        if(cols < (c1->ccx/m[0].cols)) go_left(win, m, m2, c1, (c1->ccx/m[0].cols)-cols, head);
        if(cols > (c1->ccx/m[0].cols)) go_right(win, m, m2, c1, cols-(c1->ccx/m[0].cols), head);
}
