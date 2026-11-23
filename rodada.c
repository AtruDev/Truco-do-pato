/*
 * rodada.c — Pacote 2: Rodada e vencedor da mão
 * Responsável: Pedro
 *
 * Na rodada vale a carta mais forte de cada dupla: a dupla cuja melhor
 * carta for mais forte leva. Se as duas melhores tiverem a mesma força,
 * a rodada empata (só acontece com cartas comuns).
 */
#include <stddef.h>
#include "rodada.h"
#include "manilha.h"

int apurar_rodada(Rodada *r, Valor manilha, const Mesa *m)
{
    int maior[2] = {-1, -1}; /* -1 = a dupla não tem carta na mesa */
    int i, dupla, forca;

    for (i = 0; i < r->qtd; i++) {
        if (r->quem[i] == NULL)
            continue;

        if (r->quem[i]->dupla == m->duplas[0])
            dupla = 0;
        else if (r->quem[i]->dupla == m->duplas[1])
            dupla = 1;
        else
            continue;

        forca = forca_da_carta(r->cartas[i], manilha);
        if (forca > maior[dupla])
            maior[dupla] = forca;
    }

    if (maior[0] > maior[1])
        r->dupla_vencedora = 0;
    else if (maior[1] > maior[0])
        r->dupla_vencedora = 1;
    else
        r->dupla_vencedora = RODADA_EMPATADA; /* inclui mesa vazia */

    return r->dupla_vencedora;
}

/* A carta mais forte da mesa é sempre da dupla vencedora, então basta
 * achar a primeira carta com a maior força. */
Jogador *vencedor_da_rodada(const Rodada *r, Valor manilha)
{
    int i, forca, maior = -1;
    Jogador *vencedor = NULL;

    if (r->dupla_vencedora == RODADA_EMPATADA)
        return NULL;

    for (i = 0; i < r->qtd; i++) {
        if (r->quem[i] == NULL)
            continue;
        forca = forca_da_carta(r->cartas[i], manilha);
        if (forca > maior) {
            maior = forca;
            vencedor = r->quem[i];
        }
    }
    return vencedor;
}

int apurar_mao(const int resultados[], int n)
{
    int vitorias[2] = {0, 0};
    int i;

    if (n <= 0)
        return MAO_EM_ABERTO;

    /* quem ganhar 2 rodadas leva */
    for (i = 0; i < n; i++) {
        if (resultados[i] == 0 || resultados[i] == 1) {
            vitorias[resultados[i]]++;
            if (vitorias[resultados[i]] == 2)
                return resultados[i];
        }
    }

    if (resultados[0] == RODADA_EMPATADA) {
        /* 1ª empatou: leva quem vencer a próxima que não empatar */
        for (i = 1; i < n; i++)
            if (resultados[i] != RODADA_EMPATADA)
                return resultados[i];
        return (n == 3) ? MAO_EMPATADA : MAO_EM_ABERTO;
    }

    /* alguém venceu a 1ª: se uma rodada seguinte empatou, ele leva */
    for (i = 1; i < n; i++)
        if (resultados[i] == RODADA_EMPATADA)
            return resultados[0];

    return MAO_EM_ABERTO; /* ex.: 1 rodada só, ou 0 1 (vai para a 3ª) */
}
