#ifndef FILA_H
#define FILA_H
#include <stdio.h>
#ifndef MAX_FILA
#define MAX_FILA 1005
#endif
#ifndef TP_ITEM
#define TP_ITEM
typedef int tp_item;
#endif
typedef struct { tp_item item[MAX_FILA]; int ini, fim; } tp_fila;
void inicializarFila(tp_fila *f) { f->ini = f->fim = MAX_FILA - 1; }
int vaziaFila(tp_fila *f) { return f->ini == f->fim; }
int proximoFila(int pos) { return pos == MAX_FILA - 1 ? 0 : pos + 1; }
int cheiaFila(tp_fila *f) { return proximoFila(f->fim) == f->ini; }
int insereFila(tp_fila *f, tp_item e) { if (cheiaFila(f)) return 0; f->fim = proximoFila(f->fim); f->item[f->fim] = e; return 1; }
int removeFila(tp_fila *f, tp_item *e) { if (vaziaFila(f)) return 0; f->ini = proximoFila(f->ini); *e = f->item[f->ini]; return 1; }
int peekFila(tp_fila *f, tp_item *e) { if (vaziaFila(f)) return 0; *e = f->item[proximoFila(f->ini)]; return 1; }
int tamanhoFila(tp_fila f) { int c = 0; tp_item e; while (removeFila(&f, &e)) c++; return c; }
void imprimeFila(tp_fila f) { tp_item e; while (removeFila(&f, &e)) printf("\n%d", e); }
#endif
