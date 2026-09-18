#ifndef FILA_H
#define FILA_H
#include <stdio.h>
#define MAX_FILA 6

typedef int tp_item;

typedef struct {
    tp_item item[MAX_FILA];
    int ini, fim;
} tp_fila;

void inicializarFila(tp_fila *f) {
    f->ini = f->fim = MAX_FILA - 1;
}

int vaziaFila(tp_fila *f) {
    if (f->ini == f->fim) return 1;
    return 0;
}

int proximoFila(int pos) {
    if (pos == MAX_FILA - 1) return 0;
    return ++pos;
}

int cheiaFila(tp_fila *f) {
    if (proximoFila(f->fim) == f->ini) return 1;
    return 0;
}

int insereFila(tp_fila *f, tp_item e) {
    if (cheiaFila(f)) return 0;
    f->fim = proximoFila(f->fim);
    f->item[f->fim] = e;
    return 1;
}

int removeFila(tp_fila *f, tp_item *e) {
    if (vaziaFila(f)) return 0;
    f->ini = proximoFila(f->ini);
    *e = f->item[f->ini];
    return 1;
}

int peekFila(tp_fila *f, tp_item *e) {
    if (vaziaFila(f)) return 0;
    *e = f->item[proximoFila(f->ini)];
    return 1;
}

int tamanhoFila(tp_fila f) {
    int cont = 0;
    tp_item e;
    while (!vaziaFila(&f)) {
        removeFila(&f, &e);
        cont++;
    }
    return cont;
}

void imprimeFila(tp_fila f) {
    tp_item e;
    while (!vaziaFila(&f)) {
        removeFila(&f, &e);
        printf("\n%d", e);
    }
}

#endif
