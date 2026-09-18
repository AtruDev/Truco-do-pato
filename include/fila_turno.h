/*
 * fila_turno.h — Módulo 4: Fila de turnos (roda de jogo)
 * Responsável: Joao
 *
 * Roda de jogo feita com uma FILA (TAD estruturas/fila.h), guardando a
 * cadeira na mesa (0..3) de cada jogador, na ordem em que jogam.
 * Quem joga/passa sai do início e entra no fim; o "mão" (início da fila)
 * só volta a ter a vez depois que todos os outros passaram.
 *
 * A roda é criada UMA vez por partida e reaproveitada: no fim de cada
 * mão, girar_mao() manda o "mão" para o fim e o próximo assume.
 *
 * FilaTurno é um tipo opaco (definido em fila_turno.c), porque fila.h
 * só pode ser incluído em um único .c.
 *
 * Depende de jogador.h (Jogador, Mesa).
 */
#ifndef FILA_TURNO_H
#define FILA_TURNO_H

#include "jogador.h"

typedef struct FilaTurno FilaTurno;

/* Aloca a roda e enfileira os jogadores da mesa começando pela cadeira
 * do "mão" (posicao_mao = 0..3) e seguindo a ordem da mesa.
 * Retorna NULL se faltar memória. Liberar com liberar_fila(). */
FilaTurno *criar_fila(Mesa *m, int posicao_mao);

/* Devolve o jogador da vez (início da fila), sem removê-lo.
 * Retorna NULL se a fila estiver vazia. */
Jogador *proximo_jogador(FilaTurno *f);

/* Move o jogador da vez para o fim da fila e conta a passada.
 * Retorna 1 quando todos passaram (a vez voltou ao "mão"), senão 0. */
int jogador_passou(FilaTurno *f);

/* Fim da mão: o "mão" atual vai para o fim da roda e o próximo da mesa
 * vira o novo "mão". Zera a contagem de passadas para a nova mão. */
void girar_mao(FilaTurno *f);

/* Devolve a memória da roda. */
void liberar_fila(FilaTurno *f);

#endif /* FILA_TURNO_H */
