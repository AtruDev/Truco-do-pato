/*
 * mao.h — 3ª unidade, Pacote 5 (recomendado): Módulo da mão
 * Responsável: Arthur
 *
 * Saiu do main.c sem mudar o comportamento: joga uma mão inteira (até
 * 3 rodadas), com os pedidos de truco/seis/doze, e registra cada rodada
 * no VAR. O main fica só com o menu do bar e o laço da partida.
 *
 * 3ª unidade:
 *   - jogar_mao recebe o TipoMao: na mão de onze a aposta começa
 *     bloqueada valendo 3, na mão de ferro bloqueada valendo 1;
 *   - os contadores da caderneta sobem durante a mão, pela ficha de
 *     cada jogador (trucos pedidos, recebidos e corridas);
 *   - cada rodada vai para o VAR da memória E para o historico.txt
 *     (registrar_no_var), para rever a partida depois pelo menu.
 *
 * Também ficam aqui as funções de tela e de entrada que o main usa
 * (ler_opcao, numero_da_dupla, ...), para não repetir código.
 *
 * Depende de jogador.h, fila_turno.h, rodada.h, truco.h, historico.h,
 * caderneta.h (Fregues, para os contadores) e arquivo.h (gravar_rodada).
 */
#ifndef MAO_H
#define MAO_H

#include "jogador.h"
#include "fila_turno.h"
#include "historico.h"

#define RODADAS_POR_MAO 3

typedef enum { MAO_NORMAL, MAO_DE_ONZE, MAO_DE_FERRO } TipoMao;

/* O VAR da partida atual: a lista na memória, o número da partida no
 * historico.txt e se o aviso de falha de gravação já foi dado (o aviso
 * aparece uma vez só por partida, sem precisar de variável global). */
typedef struct {
    Historico *h;
    int id;
    int avisou;
} VarPartida;

/* Registra a rodada no VAR da memória (registrar_rodada) e logo depois
 * acrescenta essa mesma rodada no historico.txt (gravar_rodada).
 * Se a gravação falhar, avisa uma vez "Atencao: nao consegui gravar o
 * VAR." e o jogo segue. Retorna 1 se as duas deram certo. */
int registrar_no_var(VarPartida *var, int mao, int rod, const Rodada *r,
                     const EventoAposta ev[], int n);

/* Avisa uma vez por partida que o VAR não foi gravado no disco. */
void avisar_falha_var(VarPartida *var);

/* Lê um número entre min e max, repetindo até a entrada ser válida
 * (recusa vazio, letras, sobras depois do número e fora da faixa).
 * Retorna o número, ou -1 se a entrada acabar (EOF). */
int ler_opcao(int min, int max);

/* Número (1 ou 2) da dupla do jogador, para exibição. */
int numero_da_dupla(const Mesa *m, const Jogador *j);

/* Índice (0 ou 1) da dupla do jogador, usado por rodada e truco. */
int indice_da_dupla(const Mesa *m, const Jogador *j);

/* Mostra as cartas que já estão na mesa nesta rodada, na ordem em que
 * saíram, ou "(vazia)". */
void exibir_cartas_na_mesa(const Mesa *m, const Rodada *r);

/* Joga uma mão inteira (até 3 rodadas) e registra cada rodada no VAR
 * (na memória e no historico.txt, com registrar_no_var).
 * tipo: MAO_NORMAL (vale 1, truco liberado), MAO_DE_ONZE (vale 3, sem
 * truco) ou MAO_DE_FERRO (vale 1, sem truco).
 * Soma nas fichas dos jogadores os trucos pedidos/recebidos e as
 * corridas (fichas NULL são ignoradas).
 * Grava em *pontos quanto a mão valeu e retorna a dupla que levou
 * (0 ou 1), ou -1 se ninguém pontuou (as 3 rodadas empataram). */
int jogar_mao(Mesa *m, FilaTurno *f, Valor manilha, VarPartida *var,
              int num_mao, TipoMao tipo, int *pontos);

#endif /* MAO_H */
