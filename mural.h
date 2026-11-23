/*
 * mural.h — 3ª unidade, Pacote 3: Mural e estatísticas
 * Responsável: Pedro
 *
 * - Mural do bar: todos os fregueses da caderneta, do que menos ganhou
 *   mãos ao que mais ganhou (somando todas as partidas). A ordenação é
 *   um MERGESORT estável sobre um vetor de ponteiros: quem empata
 *   continua em ordem alfabética.
 * - Mural da vergonha: taxa de "correu do truco" de cada freguês
 *   (correu / trucos_recebidos) e a mão mais cara da partida.
 * - Mão mais cara: a mão que deu mais pontos na partida atual
 *   (empate: fica a primeira).
 *
 * Depende de caderneta.h (Caderneta, Fregues) e jogador.h (Dupla).
 */
#ifndef MURAL_H
#define MURAL_H

#include "caderneta.h"

/* A mao que deu mais pontos na partida atual. */
typedef struct {
    int existe;               /* 0 enquanto nenhuma mao deu ponto */
    int num_mao;
    int pontos;
    int dupla;                /* 1 ou 2, para exibicao */
    char nomes[2][MAX_NOME];  /* copia dos nomes da dupla que levou */
} MaoMaisCara;

/* Ordena vet[0..n-1] por maos_ganhas, CRESCENTE, com MERGESORT.
 * Estavel: quem empata continua na ordem em que estava. */
void ordenar_por_maos(Fregues *vet[], int n);

/* correu / trucos_recebidos. Retorna -1.0 se nunca recebeu pedido. */
double taxa_correu(const Fregues *f);

/* Mostra o mural: todos os fregueses da caderneta, do que menos
 * ganhou maos para o que mais ganhou. */
void mostrar_mural(const Caderneta *c);

/* Mostra a taxa de correu de cada fregues (em ordem alfabetica) e,
 * se m != NULL, a mao mais cara da partida. */
void mostrar_mural_vergonha(const Caderneta *c, const MaoMaisCara *m);

/* Comeco de cada partida: nenhuma mao registrada. */
void iniciar_mao_mais_cara(MaoMaisCara *m);

/* Fim de cada mao: guarda esta mao se ela deu MAIS pontos que a atual
 * (empate fica a primeira). d e a dupla que levou, num_dupla 1 ou 2. */
void registrar_mao(MaoMaisCara *m, int num_mao, int pontos,
                   const Dupla *d, int num_dupla);

#endif /* MURAL_H */
