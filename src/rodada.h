/*
 * rodada.h — Pacote 2: Rodada e vencedor da mão
 * Responsável: Pedro
 *
 * Uma rodada são as cartas que foram para a mesa (até 4). apurar_rodada
 * diz qual dupla levou, usando forca_da_carta (manilha.h), e apurar_mao
 * diz se a mão (melhor de 3 rodadas) já tem dono.
 *
 * Regras do truco de verdade:
 *   - quem ganha 2 rodadas leva a mão;
 *   - 1ª empatada: leva quem vencer a próxima rodada que não empatar;
 *   - alguém venceu a 1ª e outra rodada empatou: leva quem venceu a 1ª;
 *   - as 3 empatadas: ninguém pontua (MAO_EMPATADA).
 *   - quem abre a próxima rodada é quem venceu a anterior
 *     (vencedor_da_rodada); se empatou, abre de novo quem abriu a empatada.
 *
 * Depende de jogador.h (Jogador, Mesa, Carta, Valor).
 */
#ifndef RODADA_H
#define RODADA_H

#include "jogador.h"

#define RODADA_EMPATADA  -1 /* dupla_vencedora quando a rodada empata */

/* Retornos de apurar_mao, além de 0/1 (a dupla que levou). */
#define MAO_EM_ABERTO    -1 /* ainda precisa jogar mais rodadas */
#define MAO_EMPATADA      2 /* as 3 rodadas empataram: ninguém pontua */

typedef struct {
    Carta cartas[NUM_JOGADORES];
    Jogador *quem[NUM_JOGADORES]; /* quem jogou cada carta, na ordem */
    int qtd;                      /* cartas na mesa (pode ser < 4 se correram) */
    int dupla_vencedora;          /* 0, 1 ou RODADA_EMPATADA */
} Rodada;

/* Compara a carta mais forte de cada dupla e grava o resultado em
 * r->dupla_vencedora (0, 1 ou RODADA_EMPATADA), que também é retornado.
 * Se só uma dupla tem carta na mesa, ela leva. Com manilha não existe
 * empate, porque o naipe desempata. */
int apurar_rodada(Rodada *r, Valor manilha, const Mesa *m);

/* Jogador que jogou a carta vencedora, para ele abrir a próxima rodada.
 * Chamar depois de apurar_rodada. Retorna NULL se a rodada empatou. */
Jogador *vencedor_da_rodada(const Rodada *r, Valor manilha);

/* resultados[] = vencedor de cada rodada já jogada (0, 1 ou
 * RODADA_EMPATADA), n = quantas rodadas já foram jogadas (0..3).
 * Retorna 0/1 se a mão já tem dono, MAO_EMPATADA se as 3 empataram,
 * ou MAO_EM_ABERTO se ainda precisa jogar. */
int apurar_mao(const int resultados[], int n);

#endif /* RODADA_H */
