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
};

/* Tira o jogador da vez do início da fila e o põe no fim. */
static void mover_para_o_fim(FilaTurno *f)
{
    int cadeira;

    if (removeFila(&f->fila, &cadeira))
        insereFila(&f->fila, cadeira);
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

/* Mesma operação de passar a vez, mas marcando o começo de uma mão
 * nova: o "mão" antigo vai para o fim e a contagem recomeça do zero. */
void girar_mao(FilaTurno *f)
{
    mover_para_o_fim(f);
    f->passaram = 0;
}

void liberar_fila(FilaTurno *f)
{
    free(f);
}
