/*
 * manilha.h — Módulo 3: Vira e manilha
 * Responsável: Pedro
 *
 * Revela a carta "vira", calcula qual valor vira manilha (o valor
 * seguinte ao da vira, em ciclo) e calcula a força de qualquer carta
 * levando a manilha em conta, para comparar cartas numa rodada.
 *
 * Depende de baralho.h (Carta, Valor, Naipe, Baralho).
 */
#ifndef MANILHA_H
#define MANILHA_H

#include "baralho.h"

/* Puxa a carta do topo do baralho e a devolve como vira. */
Carta revelar_vira(Baralho *b);

/* Devolve o valor seguinte ao da vira (depois do TRES volta ao QUATRO). */
Valor calcular_manilha(Carta vira);

/* Devolve um inteiro com a força da carta:
 * - carta comum: força pelo Valor (0..9);
 * - manilha: sempre maior que qualquer comum, desempatando pelo Naipe.
 * Quanto maior o número, mais forte a carta. */
int forca_da_carta(Carta c, Valor manilha);

#endif /* MANILHA_H */
