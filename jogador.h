/*
 * jogador.h — Módulo 1: Jogadores, duplas e mesa
 * Responsável: Lucca
 *
 * Define o jogador (nome + mão de cartas), a dupla (dois parceiros)
 * e a mesa (4 posições com as duplas alternadas: A1, B1, A2, B2).
 *
 * 2ª unidade: mão jogável (jogar_carta) e o menu de escolha da carta.
 * 3ª unidade: cada jogador aponta para a sua ficha na caderneta
 * (campo ficha), por onde o main soma as estatísticas durante o jogo.
 *
 * Depende de baralho.h (struct Carta).
 */
#ifndef JOGADOR_H
#define JOGADOR_H

#include "baralho.h"

#define MAX_NOME       32
#define CARTAS_NA_MAO  3
#define NUM_JOGADORES  4

struct Dupla;   /* declaração antecipada: Jogador e Dupla se referenciam */
struct Fregues; /* declaração antecipada: caderneta.h já inclui jogador.h */

typedef struct Jogador {
    char nome[MAX_NOME];
    Carta mao[CARTAS_NA_MAO];
    int qtd_cartas;        /* quantas cartas ainda estão na mão */
    struct Dupla *dupla;   /* dupla à qual o jogador pertence */
    struct Fregues *ficha; /* pagina dele na caderneta (NULL = sem ficha) */
} Jogador;

typedef struct Dupla {
    Jogador *jogadores[2];
    int pontos;
} Dupla;

typedef struct {
    Jogador *posicoes[NUM_JOGADORES]; /* ordem na mesa: dupla0, dupla1, dupla0, dupla1 */
    Dupla *duplas[2];
} Mesa;

/* Inicializa j com o nome informado (truncado em MAX_NOME), mão vazia,
 * sem dupla e sem ficha na caderneta (ficha = NULL). */
void cadastrar_jogador(Jogador *j, const char *nome);

/* Liga os dois jogadores à dupla d (e d a cada jogador) e zera os pontos. */
void criar_dupla(Dupla *d, Jogador *a, Jogador *b);

/* Preenche a mesa alternando os jogadores das duas duplas. */
void montar_mesa(Mesa *m, Dupla *d1, Dupla *d2);

/* Tira a carta da posição idx (0..qtd_cartas-1), puxa as seguintes uma casa
 * para trás e diminui qtd_cartas. Retorna 1 se deu certo, 0 se o índice
 * for inválido. Ex.: jogando a do meio, [A, B, C] vira [A, C]. */
int jogar_carta(Jogador *j, int idx, Carta *saida);

/* Mostra a mão do jogador, uma carta por linha, numerada a partir de 1. */
void exibir_mao(const Jogador *j);

/* Mostra a mão e lê qual carta jogar, repetindo até a entrada ser válida
 * (rejeita linha vazia, letras e número fora da faixa).
 * Retorna o índice escolhido (0..qtd_cartas-1), ou -1 se a mão estiver
 * vazia ou a entrada acabar (EOF). Quem chama precisa tratar o -1. */
int escolher_carta(const Jogador *j);

#endif /* JOGADOR_H */
