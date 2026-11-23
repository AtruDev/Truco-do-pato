/*
 * manilha.c — Módulo 3: Vira e manilha
 * Responsável: Pedro
 *
 * A vira é a carta tirada do topo depois da distribuição. A manilha é
 * o valor seguinte ao da vira na sequência cíclica
 * 4 -> 5 -> 6 -> 7 -> Q -> J -> K -> A -> 2 -> 3 -> (volta ao 4).
 * Como o enum Valor já está nessa ordem, basta somar 1 com módulo.
 */
#include "manilha.h"

/* Tira a vira do topo do baralho (nunca do meio). */
Carta revelar_vira(Baralho *b)
{
    Carta vira = {0};

    puxar_do_topo(b, &vira);
    return vira;
}

/* Valor seguinte ao da vira; depois do TRES (9) volta ao QUATRO (0). */
Valor calcular_manilha(Carta vira)
{
    return (Valor) ((vira.valor + 1) % NUM_VALORES);
}

/* Força numérica da carta, para comparar numa rodada:
 * - carta comum: o próprio Valor, de 0 (4) a 9 (3); o naipe não conta;
 * - manilha: NUM_VALORES + naipe, ou seja, de 10 (ouros) a 13 (paus, o zap),
 *   sempre acima de qualquer carta comum. */
int forca_da_carta(Carta c, Valor manilha)
{
    if (c.valor == manilha)
        return NUM_VALORES + c.naipe;

    return c.valor;
}
