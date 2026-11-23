/*
 * historico.h — Pacote 4: VAR (histórico navegável)
 *
 * O VAR é uma lista duplamente encadeada com um nó por rodada, alocado
 * dinamicamente, porque ninguém sabe quantas rodadas a partida vai ter.
 * (O listase.h não serve: ele só anda para um lado.)
 *
 * Cada nó guarda CÓPIAS (a rodada e os nomes por valor): as mãos dos
 * jogadores são sobrescritas a cada distribuição, então um ponteiro
 * para elas faria o VAR mostrar a carta errada.
 *
 * A lista é uma só para a partida inteira, então a navegação atravessa
 * de uma mão para a outra sem tratamento especial.
 *
 * Historico é um tipo opaco (definido em historico.c).
 *
 * 3ª unidade (VAR em disco): ultima_rodada dá o nó recém-registrado
 * para o arquivo.c gravar, e registrar_no põe um nó lido do arquivo
 * de volta na lista (partidas antigas, sem Jogador).
 *
 * Depende de rodada.h (Rodada, Jogador, MAX_NOME).
 */
#ifndef HISTORICO_H
#define HISTORICO_H

#include "rodada.h"

/* No máximo: truco, aceitou, seis, aceitou, doze, aceitou. */
#define MAX_EVENTOS 6

typedef enum {
    PEDIU_TRUCO,
    PEDIU_SEIS,
    PEDIU_DOZE,
    ACEITOU,
    CORREU
} TipoEvento;

typedef struct {
    int dupla;              /* quem agiu (0 ou 1) */
    char nome[MAX_NOME];    /* nome de quem agiu (cópia) */
    int tipo;               /* um TipoEvento */
} EventoAposta;

typedef struct NoHist {
    int num_mao, num_rodada;
    Rodada rodada;                              /* cópia, não ponteiro */
    char nomes[NUM_JOGADORES][MAX_NOME];        /* quem jogou cada carta (cópia) */
    EventoAposta eventos[MAX_EVENTOS];
    int qtd_eventos;
    struct NoHist *ant, *prox;
} NoHist;

typedef struct Historico Historico;

/* Lista vazia. Retorna NULL se faltar memória. */
Historico *criar_historico(void);

/* Monta um evento de aposta para passar a registrar_rodada (copia o nome). */
EventoAposta criar_evento(int dupla, const char *nome, TipoEvento tipo);

/* Põe um nó no fim da lista com cópias da rodada r e dos n eventos de
 * aposta dela (n > MAX_EVENTOS é cortado). Se alguém correu no meio,
 * r pode ter menos de 4 cartas.
 * Retorna 1 se deu certo, 0 se faltar memória. */
int registrar_rodada(Historico *h, int mao, int rod, const Rodada *r,
                     const EventoAposta ev[], int n);

/* Ultimo no da lista (a rodada recem-registrada), ou NULL se vazia.
 * Usado para gravar a rodada no arquivo. */
const NoHist *ultima_rodada(const Historico *h);

/* Versao 2 (grafica): primeiro no da lista, ou NULL se vazia. A tela do
 * VAR anda pela lista com ->prox / ->ant a partir dele, sem imprimir. */
const NoHist *primeira_rodada(const Historico *h);

/* Poe no fim uma copia de 'dados' (numeros, cartas, nomes e eventos),
 * sem precisar de Jogador: usado ao carregar partida antiga.
 * rodada.quem fica NULL (exibir_no so usa os nomes copiados).
 * Retorna 1 ou 0. */
int registrar_no(Historico *h, const NoHist *dados);

/* Quantas rodadas o VAR tem. */
int tamanho_historico(const Historico *h);

/* Menu [a]vançar [v]oltar [s]air, começando pela rodada mais recente.
 * Avisa quando tenta passar do começo ou do fim. */
void navegar_var(const Historico *h);

/* Libera todos os nós e o próprio histórico. */
void liberar_historico(Historico *h);

#endif /* HISTORICO_H */
