/*
 * baralho.h — Módulo 2: Baralho
 * Responsável: Rafael
 *
 * Define a carta e o baralho de truco (40 cartas: sem 8, 9, 10 e coringas).
 * O baralho é uma PILHA (TAD estruturas/pilha.h): só sai carta pelo topo.
 *
 * Baralho é um tipo opaco: os outros módulos só têm um ponteiro para ele
 * e não enxergam o que tem dentro. Assim ninguém consegue pegar uma carta
 * do meio; a única saída é puxar_do_topo().
 *
 * As structs e enums abaixo são usadas por TODOS os módulos.
 * Não altere sem combinar com o grupo.
 */
#ifndef BARALHO_H
#define BARALHO_H

#define TAM_BARALHO 40

/* Ordem dos naipes = força quando a carta é manilha (ouros < ... < paus). */
typedef enum {
    OUROS = 0,
    ESPADAS,
    COPAS,
    PAUS,
    NUM_NAIPES
} Naipe;

/* Ordem dos valores = força normal no truco (4 < 5 < ... < 2 < 3). */
typedef enum {
    QUATRO = 0,
    CINCO,
    SEIS,
    SETE,
    DAMA,   /* Q */
    VALETE, /* J */
    REI,    /* K */
    AS,
    DOIS,
    TRES,
    NUM_VALORES
} Valor;

typedef struct {
    Naipe naipe;
    Valor valor;
} Carta;

/* Tipo opaco: a definição fica em baralho.c. */
typedef struct Baralho Baralho;

/* Aloca um baralho com as 40 cartas em ordem.
 * Retorna NULL se faltar memória. Liberar com liberar_baralho(). */
Baralho *criar_baralho(void);

/* Recolhe as 40 cartas, embaralha com Fisher-Yates e empilha de novo.
 * (o srand deve ser chamado uma única vez no main) */
void embaralhar(Baralho *b);

/* Desempilha a carta do topo para *saida.
 * Retorna 1 em caso de sucesso, 0 se o baralho estiver vazio. */
int puxar_do_topo(Baralho *b, Carta *saida);

/* Devolve a memória do baralho. */
void liberar_baralho(Baralho *b);

#endif /* BARALHO_H */
