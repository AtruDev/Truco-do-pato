/*
 * truco.h — Pacote 3: Truco (máquina de estados da aposta)
 *
 * Controla quanto a mão vale (1, 3, 6 ou 12) e quem pode pedir ou
 * responder em cada momento. Não sabe nada de cartas nem de jogadores:
 * só de duplas (0 ou 1).
 *
 * Estados (nivel): 0 = nada pedido, 1 = truco, 2 = seis, 3 = doze.
 * Cada degrau é pedido uma vez só, sempre na ordem truco -> seis -> doze.
 *
 *   pedir(a, d)               -> a dupla d pede o próximo degrau
 *   responder(a, ACEITAR)     -> a mão passa a valer o degrau pedido
 *   responder(a, AUMENTAR)    -> aceita e já pede o próximo degrau; agora
 *                                quem responde é a dupla que pediu antes
 *   responder(a, CORRER)      -> a mão acaba; quem pediu ganha o valor
 *                                anterior ao pedido (truco 1, seis 3, doze 6)
 *
 * Quem responde é sempre a dupla adversária de a->dupla_pediu
 * (dupla_que_responde). A mão de 12 encerrar a partida fica com o main.
 *
 * 3ª unidade (mão de onze): a aposta pode começar BLOQUEADA, valendo
 * um valor fixo (3 na mão de onze, 1 na mão de ferro). Com bloqueada = 1
 * ninguém pode pedir (pode_pedir retorna 0 logo no começo).
 */
#ifndef TRUCO_H
#define TRUCO_H

#define NIVEL_MAXIMO 3     /* doze */
#define RESPOSTA_INVALIDA -1

typedef enum { ACEITAR, CORRER, AUMENTAR } Resposta;

typedef struct {
    int nivel;       /* 0=nada, 1=truco, 2=seis, 3=doze */
    int valor;       /* 1, 3, 6, 12: o que a mão vale agora */
    int dupla_pediu; /* quem fez o último pedido (-1 = ninguém) */
    int aguardando;  /* 1 = tem pedido sem resposta */
    int bloqueada;   /* 1 = ninguem pode pedir nesta mao */
} Aposta;

/* Começo de cada mão: nada pedido, a mão vale 1, pedidos liberados. */
void iniciar_aposta(Aposta *a);

/* Mao de onze (valor 3) ou de ferro (valor 1): a mao comeca valendo
 * 'valor' e ninguem pode pedir. iniciar_aposta passa a zerar bloqueada. */
void iniciar_aposta_bloqueada(Aposta *a, int valor);

/* 1 se a dupla pode pedir o próximo degrau agora: a aposta não está
 * bloqueada (mão de onze/ferro), não há pedido esperando resposta, ainda não chegou no doze e não foi ela quem
 * fez o último pedido aceito. */
int pode_pedir(const Aposta *a, int dupla);

/* A dupla pede o próximo degrau. Não faz nada se !pode_pedir(a, dupla). */
void pedir(Aposta *a, int dupla);

/* Responde o pedido pendente.
 * CORRER: retorna os pontos que vão para quem pediu (1, 3 ou 6).
 * ACEITAR / AUMENTAR: retorna 0.
 * Retorna RESPOSTA_INVALIDA (sem mudar nada) se não há pedido pendente
 * ou se tentou AUMENTAR no doze. */
int responder(Aposta *a, Resposta r);

/* Dupla que deve responder o pedido pendente (-1 se não há pedido). */
int dupla_que_responde(const Aposta *a);

/* Nome do degrau para exibição: "truco", "seis" ou "doze". */
const char *nome_nivel(int nivel);

#endif /* TRUCO_H */
