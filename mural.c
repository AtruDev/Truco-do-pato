/*
 * mural.c — 3ª unidade, Pacote 3: Mural e estatísticas
 * Responsável: Pedro
 *
 * O mergesort ordena um vetor de PONTEIROS (as fichas não são copiadas)
 * e usa um vetor auxiliar alocado uma vez só, no ordenar_por_maos.
 * Na intercalação, em caso de empate, entra primeiro o da esquerda
 * (<=): é isso que deixa o algoritmo estável. Como o vetor chega em
 * ordem alfabética (caderneta_para_vetor), os empatados continuam em
 * ordem alfabética.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mural.h"

/* ------------------------------------------------------------------ */
/* Mergesort                                                           */
/* ------------------------------------------------------------------ */

/* Intercala vet[ini..meio] e vet[meio+1..fim] (já ordenados) usando aux. */
static void intercalar(Fregues *vet[], Fregues *aux[], int ini, int meio,
                       int fim)
{
    int i = ini, j = meio + 1, k = ini;

    while (i <= meio && j <= fim) {
        /* <= : no empate fica o da esquerda primeiro (estável) */
        if (vet[i]->maos_ganhas <= vet[j]->maos_ganhas)
            aux[k++] = vet[i++];
        else
            aux[k++] = vet[j++];
    }
    while (i <= meio)
        aux[k++] = vet[i++];
    while (j <= fim)
        aux[k++] = vet[j++];

    for (k = ini; k <= fim; k++)
        vet[k] = aux[k];
}

/* Divide ao meio, ordena cada metade e intercala. */
static void mergesort_rec(Fregues *vet[], Fregues *aux[], int ini, int fim)
{
    int meio;

    if (ini >= fim)
        return;
    meio = ini + (fim - ini) / 2;
    mergesort_rec(vet, aux, ini, meio);
    mergesort_rec(vet, aux, meio + 1, fim);
    intercalar(vet, aux, ini, meio, fim);
}

void ordenar_por_maos(Fregues *vet[], int n)
{
    Fregues **aux;

    if (vet == NULL || n < 2)
        return;
    aux = malloc((size_t) n * sizeof(Fregues *));
    if (aux == NULL) {
        printf("Erro: memoria insuficiente para ordenar o mural.\n");
        return;
    }
    mergesort_rec(vet, aux, 0, n - 1);
    free(aux);
}

/* ------------------------------------------------------------------ */
/* Estatísticas                                                        */
/* ------------------------------------------------------------------ */

double taxa_correu(const Fregues *f)
{
    if (f->trucos_recebidos <= 0)
        return -1.0;
    return (double) f->correu / f->trucos_recebidos;
}

/* Vetor com todas as fichas em ordem alfabética. Devolve NULL se a
 * caderneta estiver vazia ou faltar memória (*n recebe a quantidade). */
static Fregues **vetor_da_caderneta(const Caderneta *c, int *n)
{
    Fregues **vet;

    *n = 0;
    if (c->quantidade <= 0)
        return NULL;
    vet = malloc((size_t) c->quantidade * sizeof(Fregues *));
    if (vet == NULL) {
        printf("Erro: memoria insuficiente para montar o mural.\n");
        return NULL;
    }
    *n = caderneta_para_vetor(c, vet, c->quantidade);
    return vet;
}

void mostrar_mural(const Caderneta *c)
{
    Fregues **vet;
    int n, i;

    printf("\n======= Mural do Bar do Pato =======\n");
    if (c->quantidade == 0) {
        printf("Ninguem sentou na mesa ainda.\n");
        return;
    }
    vet = vetor_da_caderneta(c, &n);
    if (vet == NULL)
        return;

    ordenar_por_maos(vet, n);

    printf("%2s  %-20s%11s%10s\n", "#", "Nome", "Maos ganhas", "Partidas");
    for (i = 0; i < n; i++)
        printf("%2d  %-20s%11d%10d\n", i + 1, vet[i]->nome,
               vet[i]->maos_ganhas, vet[i]->partidas);
    free(vet);
}

void mostrar_mural_vergonha(const Caderneta *c, const MaoMaisCara *m)
{
    Fregues **vet;
    char fracao[24];
    double taxa;
    int n, i;

    printf("\n======= Mural da vergonha =======\n");
    if (c->quantidade == 0) {
        printf("Ninguem sentou na mesa ainda.\n");
    } else {
        vet = vetor_da_caderneta(c, &n);   /* já em ordem alfabética */
        if (vet != NULL) {
            printf("%-20s%14s%7s\n", "Nome", "Correu/Recebeu", "Taxa");
            for (i = 0; i < n; i++) {
                snprintf(fracao, sizeof fracao, "%d/%d", vet[i]->correu,
                         vet[i]->trucos_recebidos);
                taxa = taxa_correu(vet[i]);
                if (taxa < 0)
                    printf("%-20s%14s%7s\n", vet[i]->nome, fracao, "--");
                else
                    printf("%-20s%14s%6.1f%%\n", vet[i]->nome, fracao,
                           taxa * 100.0);
            }
            free(vet);
        }
    }

    if (m == NULL)
        return;
    if (!m->existe)
        printf("\nNenhuma mao valeu ponto.\n");
    else
        printf("\nMao mais cara da partida: mao %d, %d ponto%s, dupla %d "
               "(%s e %s)\n", m->num_mao, m->pontos, m->pontos == 1 ? "" : "s",
               m->dupla, m->nomes[0], m->nomes[1]);
}

/* ------------------------------------------------------------------ */
/* Mão mais cara                                                       */
/* ------------------------------------------------------------------ */

void iniciar_mao_mais_cara(MaoMaisCara *m)
{
    memset(m, 0, sizeof *m);
    m->existe = 0;
}

void registrar_mao(MaoMaisCara *m, int num_mao, int pontos,
                   const Dupla *d, int num_dupla)
{
    int i;

    if (d == NULL || pontos <= 0)
        return;
    /* estritamente maior: no empate fica a primeira */
    if (m->existe && pontos <= m->pontos)
        return;

    m->existe = 1;
    m->num_mao = num_mao;
    m->pontos = pontos;
    m->dupla = num_dupla;
    for (i = 0; i < 2; i++) {
        strncpy(m->nomes[i], d->jogadores[i]->nome, MAX_NOME - 1);
        m->nomes[i][MAX_NOME - 1] = '\0';
    }
}
