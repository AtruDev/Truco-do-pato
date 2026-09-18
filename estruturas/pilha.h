#ifndef PILHA_H
#define PILHA_H
#include <stdio.h>
#define MAX_PILHA 100

typedef int tp_item;

typedef struct {
    tp_item item[MAX_PILHA];
    int topo;
} tp_pilha;

void inicializarPilha(tp_pilha *p) {
    p->topo = -1;
}

int vaziaPilha(tp_pilha *p) {
    if (p->topo == -1) return 1;
    return 0;
}

int cheiaPilha(tp_pilha *p) {
    if (p->topo == MAX_PILHA - 1) return 1;
    return 0;
}

int inserePilha(tp_pilha *p, tp_item e) {
    if (cheiaPilha(p)) return 0;
    p->topo++;
    p->item[p->topo] = e;
    return 1;
}

int removePilha(tp_pilha *p, tp_item *e) {
    if (vaziaPilha(p)) return 0;
    *e = p->item[p->topo];
    p->topo--;
    return 1;
}

int peekPilha(tp_pilha *p, tp_item *e) {
    if (vaziaPilha(p)) return 0;
    *e = p->item[p->topo];
    return 1;
}

int tamanhoPilha(tp_pilha *p) {
    return p->topo + 1;
}

void imprimePilha(tp_pilha p) {
    tp_item e;
    while (!vaziaPilha(&p)) {
        removePilha(&p, &e);
        printf("\n%d", e);
    }
}

#endif
