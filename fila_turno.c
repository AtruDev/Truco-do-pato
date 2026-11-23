/*
 * fila_turno.c — Módulo 4: Fila de turnos (roda de jogo)
 * Responsável: Joao
 *
 * A roda é uma FILA CIRCULAR, usando o TAD estruturas/fila.h. Na fila
 * vai a cadeira de cada jogador na mesa (o tp_item do TAD é int), e a
 * Mesa converte a cadeira no Jogador.
 *
 * "Passar a vez" é tirar do início (removeFila) e pôr no fim (insereFila).
 *
 * fila.h traz as funções definidas no próprio header, então ele só pode
 * ser incluído neste arquivo. Por isso FilaTurno é opaco.
 */
#include <stdlib.h>
#include "fila_turno.h"
#include "fila.h"

struct FilaTurno {
    tp_fila fila;   /* cadeiras (0..3) na ordem da vez; início = da vez */
    Mesa *mesa;     /* para converter a cadeira no Jogador */
    int passaram;   /* quantos já passaram desde que o "mão" jogou */
    int cadeira_mao; /* cadeira do "mão" desta mão (não muda entre rodadas) */
};

/* Tira o jogador da vez do início da fila e o põe no fim. */
static void mover_para_o_fim(FilaTurno *f)
{
    int cadeira;

    if (removeFila(&f->fila, &cadeira))
        insereFila(&f->fila, cadeira);
}

/* Gira a roda até a cadeira indicada ficar no início.
 * Retorna 1 se achou, 0 se a cadeira não está na fila. */
static int girar_ate(FilaTurno *f, int cadeira)
{
    int i, atual;

    for (i = 0; i < NUM_JOGADORES; i++) {
        if (peekFila(&f->fila, &atual) && atual == cadeira)
            return 1;
        mover_para_o_fim(f);
    }
    return 0;
}

/* Enfileira as cadeiras a partir do "mão".
 * Ex.: posicao_mao = 1 -> cadeiras 1, 2, 3, 0. */
FilaTurno *criar_fila(Mesa *m, int posicao_mao)
{
    FilaTurno *f = malloc(sizeof(FilaTurno));
    int i;

    if (f == NULL)
        return NULL;

    inicializarFila(&f->fila);
    for (i = 0; i < NUM_JOGADORES; i++)
        insereFila(&f->fila, (posicao_mao + i) % NUM_JOGADORES);

    f->mesa = m;
    f->passaram = 0;
    f->cadeira_mao = posicao_mao;
    return f;
}

/* Consulta quem joga agora (peekFila), sem tirar ninguém da fila. */
Jogador *proximo_jogador(FilaTurno *f)
{
    int cadeira;

    if (!peekFila(&f->fila, &cadeira))
        return NULL;

    return f->mesa->posicoes[cadeira];
}

/* O jogador da vez vai para o fim. Depois de uma volta completa, avisa
 * que a vez voltou ao "mão" e recomeça a contagem. */
int jogador_passou(FilaTurno *f)
{
    if (vaziaFila(&f->fila))
        return 0;

    mover_para_o_fim(f);
    f->passaram++;

    if (f->passaram == NUM_JOGADORES) {
        f->passaram = 0;
        return 1; /* a vez voltou ao "mão" */
    }
    return 0;
}

/* Começo de uma mão nova: volta a roda para o "mão" desta mão (ela
 * pode ter sido girada por fila_posicionar), manda ele para o fim e
 * o próximo vira o novo "mão". A contagem recomeça do zero. */
void girar_mao(FilaTurno *f)
{
    girar_ate(f, f->cadeira_mao);
    mover_para_o_fim(f);
    peekFila(&f->fila, &f->cadeira_mao);
    f->passaram = 0;
}

/* Acha a cadeira do jogador na mesa e gira a roda até ela. */
int fila_posicionar(FilaTurno *f, Jogador *j)
{
    int cadeira;

    for (cadeira = 0; cadeira < NUM_JOGADORES; cadeira++)
        if (f->mesa->posicoes[cadeira] == j)
            break;

    if (cadeira == NUM_JOGADORES || !girar_ate(f, cadeira))
        return 0;

    f->passaram = 0;
    return 1;
}

void liberar_fila(FilaTurno *f)
{
    free(f);
}
