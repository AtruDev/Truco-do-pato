/*
 * jogador.h — Módulo 1: Jogadores, duplas e mesa
 * Responsável: Lucas
 *
 * Define o jogador (nome + mão de cartas), a dupla (dois parceiros)
 * e a mesa (4 posições com as duplas alternadas: A1, B1, A2, B2).
 *
 * Depende de baralho.h (struct Carta).
 */
#ifndef JOGADOR_H
#define JOGADOR_H

#include "baralho.h"

#define MAX_NOME       32
#define CARTAS_NA_MAO  3
#define NUM_JOGADORES  4

struct Dupla; /* declaração antecipada: Jogador e Dupla se referenciam */

typedef struct Jogador {
    char nome[MAX_NOME];
    Carta mao[CARTAS_NA_MAO];
    int qtd_cartas;        /* quantas cartas ainda estão na mão */
    struct Dupla *dupla;   /* dupla à qual o jogador pertence */
} Jogador;

typedef struct Dupla {
    Jogador *jogadores[2];
    int pontos;
} Dupla;

typedef struct {
    Jogador *posicoes[NUM_JOGADORES]; /* ordem na mesa: dupla0, dupla1, dupla0, dupla1 */
    Dupla *duplas[2];
} Mesa;

/* Inicializa j com o nome informado (truncado em MAX_NOME), mão vazia
 * e sem dupla. */
void cadastrar_jogador(Jogador *j, const char *nome);

/* Liga os dois jogadores à dupla d (e d a cada jogador) e zera os pontos. */
void criar_dupla(Dupla *d, Jogador *a, Jogador *b);

/* Preenche a mesa alternando os jogadores das duas duplas. */
void montar_mesa(Mesa *m, Dupla *d1, Dupla *d2);

#endif /* JOGADOR_H */
