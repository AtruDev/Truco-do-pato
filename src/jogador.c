/*
 * jogador.c — Módulo 1: Jogadores, duplas e mesa
 * Responsável: Lucca
 *
 * Cada Jogador carrega junto tudo o que é dele: nome, cartas na mão
 * e um ponteiro para a própria dupla. A Dupla aponta de volta para os
 * dois parceiros, e a Mesa guarda a ordem em que eles estão sentados.
 */
#include <stddef.h>
#include <string.h>
#include "jogador.h"

/* Copia o nome (no máximo MAX_NOME - 1 caracteres, sempre com '\0'),
 * deixa a mão vazia e o jogador ainda sem dupla. */
void cadastrar_jogador(Jogador *j, const char *nome)
{
    strncpy(j->nome, nome, MAX_NOME - 1);
    j->nome[MAX_NOME - 1] = '\0'; /* strncpy não garante o terminador */
    j->qtd_cartas = 0;
    j->dupla = NULL;
}

/* Liga os dois lados: a dupla conhece seus jogadores e cada jogador
 * conhece sua dupla. A dupla começa com 0 pontos. */
void criar_dupla(Dupla *d, Jogador *a, Jogador *b)
{
    d->jogadores[0] = a;
    d->jogadores[1] = b;
    d->pontos = 0;
    a->dupla = d;
    b->dupla = d;
}

/* Senta os jogadores alternando as duplas: D1, D2, D1, D2.
 * Assim, seguindo a mesa em ordem, nunca jogam dois parceiros seguidos. */
void montar_mesa(Mesa *m, Dupla *d1, Dupla *d2)
{
    m->duplas[0] = d1;
    m->duplas[1] = d2;

    m->posicoes[0] = d1->jogadores[0];
    m->posicoes[1] = d2->jogadores[0];
    m->posicoes[2] = d1->jogadores[1];
    m->posicoes[3] = d2->jogadores[1];
}
