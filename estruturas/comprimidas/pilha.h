#ifndef PILHA_H
#define PILHA_H
#include <stdio.h>
#ifndef MAX_PILHA
#define MAX_PILHA 1005
#endif
#ifndef TP_ITEM
#define TP_ITEM
typedef int tp_item;
#endif
typedef struct { tp_item item[MAX_PILHA]; int topo; } tp_pilha;
void inicializarPilha(tp_pilha *p) { p->topo = -1; }
int vaziaPilha(tp_pilha *p) { return p->topo == -1; }
int cheiaPilha(tp_pilha *p) { return p->topo == MAX_PILHA - 1; }
int inserePilha(tp_pilha *p, tp_item e) { if (cheiaPilha(p)) return 0; p->item[++p->topo] = e; return 1; }
int removePilha(tp_pilha *p, tp_item *e) { if (vaziaPilha(p)) return 0; *e = p->item[p->topo--]; return 1; }
int peekPilha(tp_pilha *p, tp_item *e) { if (vaziaPilha(p)) return 0; *e = p->item[p->topo]; return 1; }
int tamanhoPilha(tp_pilha *p) { return p->topo + 1; }
void imprimePilha(tp_pilha p) { tp_item e; while (removePilha(&p, &e)) printf("\n%d", e); }
#endif
