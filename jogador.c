/*
 * jogador.c — Módulo 1: Jogadores, duplas e mesa
 * Responsável: Lucca
 *
 * Cada Jogador carrega junto tudo o que é dele: nome, cartas na mão
 * e um ponteiro para a própria dupla. A Dupla aponta de volta para os
 * dois parceiros, e a Mesa guarda a ordem em que eles estão sentados.
 *
 * 2ª unidade: a mão é um vetor sem buracos. Jogar uma carta tira ela
 * da posição escolhida e puxa as seguintes uma casa para trás.
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jogador.h"

#define TAM_LINHA 64

/* Copia o nome (no máximo MAX_NOME - 1 caracteres, sempre com '\0'),
 * deixa a mão vazia e o jogador ainda sem dupla e sem ficha. */
void cadastrar_jogador(Jogador *j, const char *nome)
{
    strncpy(j->nome, nome, MAX_NOME - 1);
    j->nome[MAX_NOME - 1] = '\0'; /* strncpy não garante o terminador */
    j->qtd_cartas = 0;
    j->dupla = NULL;
    j->ficha = NULL; /* o main liga à caderneta depois do cadastro */
}

/* Liga os dois lados: a dupla conhece seus jogadores e cada jogador
 * conhece sua dupla. A dupla começa com 0 pontos. */
void criar_dupla(Dupla *d, Jogador *a, Jogador *b)
{
    d->jogadores[0] = a;
    d->jogadores[1] = b;
    d->pontos = 0;
    a->dupla = d;
    b->dupla = d;
}

/* Senta os jogadores alternando as duplas: D1, D2, D1, D2.
 * Assim, seguindo a mesa em ordem, nunca jogam dois parceiros seguidos. */
void montar_mesa(Mesa *m, Dupla *d1, Dupla *d2)
{
    m->duplas[0] = d1;
    m->duplas[1] = d2;

    m->posicoes[0] = d1->jogadores[0];
    m->posicoes[1] = d2->jogadores[0];
    m->posicoes[2] = d1->jogadores[1];
    m->posicoes[3] = d2->jogadores[1];
}

/* Copia a carta escolhida para *saida e fecha o buraco puxando as
 * cartas seguintes uma posição para trás. */
int jogar_carta(Jogador *j, int idx, Carta *saida)
{
    int i;

    if (idx < 0 || idx >= j->qtd_cartas)
        return 0; /* índice inválido (inclui a mão vazia) */

    *saida = j->mao[idx];

    for (i = idx; i < j->qtd_cartas - 1; i++)
        j->mao[i] = j->mao[i + 1];

    j->qtd_cartas--;
    return 1;
}

void exibir_mao(const Jogador *j)
{
    int i;

    for (i = 0; i < j->qtd_cartas; i++) {
        printf("  %d) ", i + 1);
        mostrar_carta(j->mao[i]);
        printf("\n");
    }
}

/* Lê a linha inteira com fgets e converte com strtol, para conseguir
 * recusar letras, sobras depois do número e linhas vazias. */
int escolher_carta(const Jogador *j)
{
    char linha[TAM_LINHA];
    char *fim;
    size_t n;
    long escolha;
    int ch;

    if (j->qtd_cartas <= 0)
        return -1;

    for (;;) {
        printf("\n%s, suas cartas:\n", j->nome);
        exibir_mao(j);
        printf("Escolha a carta (1 a %d): ", j->qtd_cartas);
        fflush(stdout);

        if (fgets(linha, sizeof linha, stdin) == NULL)
            return -1; /* a entrada acabou */

        n = strlen(linha);
        if (n > 0 && linha[n - 1] == '\n') {
            linha[n - 1] = '\0';
        } else if (n == sizeof linha - 1) {
            /* linha maior que o buffer: descarta o resto e pede de novo */
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("Entrada muito longa. Digite so o numero da carta.\n");
            continue;
        }

        escolha = strtol(linha, &fim, 10);

        if (fim == linha) { /* nenhum dígito: vazia ou letra */
            printf("Entrada invalida. Digite o numero da carta.\n");
            continue;
        }
        while (*fim == ' ' || *fim == '\t')
            fim++;
        if (*fim != '\0') { /* sobrou algo depois do número, ex.: "2x" */
            printf("Entrada invalida. Digite apenas o numero.\n");
            continue;
        }
        if (escolha < 1 || escolha > j->qtd_cartas) {
            printf("Numero fora da faixa. Escolha entre 1 e %d.\n",
                   j->qtd_cartas);
            continue;
        }

        return (int) (escolha - 1); /* o menu mostra 1..n, o vetor é 0..n-1 */
    }
}
