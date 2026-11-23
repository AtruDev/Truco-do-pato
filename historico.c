/*
 * historico.c — Pacote 4: VAR (histórico navegável)
 *
 * Lista duplamente encadeada com descritor: o Historico guarda o
 * primeiro e o último nó e a quantidade. Inserir é sempre no fim, O(1).
 * Navegar é seguir ant/prox a partir do nó atual.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"

struct Historico {
    NoHist *inicio, *fim;
    int qtd;
};

/* strncpy que sempre termina com '\0' */
static void copiar_nome(char *destino, const char *origem)
{
    strncpy(destino, origem != NULL ? origem : "", MAX_NOME - 1);
    destino[MAX_NOME - 1] = '\0';
}

Historico *criar_historico(void)
{
    Historico *h = malloc(sizeof(Historico));

    if (h == NULL)
        return NULL;
    h->inicio = h->fim = NULL;
    h->qtd = 0;
    return h;
}

EventoAposta criar_evento(int dupla, const char *nome, TipoEvento tipo)
{
    EventoAposta e;

    e.dupla = dupla;
    copiar_nome(e.nome, nome);
    e.tipo = tipo;
    return e;
}

int registrar_rodada(Historico *h, int mao, int rod, const Rodada *r,
                     const EventoAposta ev[], int n)
{
    NoHist *no;
    int i;

    if (h == NULL || r == NULL)
        return 0;

    no = malloc(sizeof(NoHist));
    if (no == NULL)
        return 0;

    no->num_mao = mao;
    no->num_rodada = rod;
    no->rodada = *r; /* cópia da struct inteira, cartas incluídas */

    for (i = 0; i < NUM_JOGADORES; i++)
        copiar_nome(no->nomes[i],
                    (i < r->qtd && r->quem[i] != NULL) ? r->quem[i]->nome : "");

    if (n > MAX_EVENTOS)
        n = MAX_EVENTOS;
    if (n < 0 || ev == NULL)
        n = 0;
    for (i = 0; i < n; i++)
        no->eventos[i] = ev[i];
    no->qtd_eventos = n;

    /* encadeia no fim */
    no->prox = NULL;
    no->ant = h->fim;
    if (h->fim != NULL)
        h->fim->prox = no;
    else
        h->inicio = no;
    h->fim = no;
    h->qtd++;
    return 1;
}

const NoHist *ultima_rodada(const Historico *h)
{
    return h != NULL ? h->fim : NULL;
}

/* Copia o nó inteiro e acerta o que não pode vir de fora: os ponteiros
 * para jogadores (que não existem numa partida antiga), os limites de
 * qtd e os terminadores dos nomes. Depois encadeia no fim. */
int registrar_no(Historico *h, const NoHist *dados)
{
    NoHist *no;
    int i;

    if (h == NULL || dados == NULL)
        return 0;

    no = malloc(sizeof(NoHist));
    if (no == NULL)
        return 0;

    *no = *dados;
    if (no->rodada.qtd < 0 || no->rodada.qtd > NUM_JOGADORES)
        no->rodada.qtd = 0;
    if (no->qtd_eventos < 0 || no->qtd_eventos > MAX_EVENTOS)
        no->qtd_eventos = 0;
    for (i = 0; i < NUM_JOGADORES; i++) {
        no->rodada.quem[i] = NULL;
        no->nomes[i][MAX_NOME - 1] = '\0';
    }
    for (i = 0; i < no->qtd_eventos; i++)
        no->eventos[i].nome[MAX_NOME - 1] = '\0';

    no->prox = NULL;
    no->ant = h->fim;
    if (h->fim != NULL)
        h->fim->prox = no;
    else
        h->inicio = no;
    h->fim = no;
    h->qtd++;
    return 1;
}

int tamanho_historico(const Historico *h)
{
    return h != NULL ? h->qtd : 0;
}

static const char *texto_evento(int tipo)
{
    switch (tipo) {
    case PEDIU_TRUCO: return "pediu truco";
    case PEDIU_SEIS:  return "pediu seis";
    case PEDIU_DOZE:  return "pediu doze";
    case ACEITOU:     return "aceitou";
    case CORREU:      return "correu";
    }
    return "?";
}

/* Mostra um nó: cartas na ordem em que foram jogadas, apostas e resultado. */
static void exibir_no(const NoHist *no, int posicao, int total)
{
    int i, correu = 0;

    printf("\n----- VAR %d/%d: mao %d, rodada %d -----\n",
           posicao, total, no->num_mao, no->num_rodada);

    if (no->rodada.qtd == 0)
        printf("  (nenhuma carta jogada)\n");
    for (i = 0; i < no->rodada.qtd; i++) {
        printf("  %-15s ", no->nomes[i]);
        mostrar_carta(no->rodada.cartas[i]);
        printf("\n");
    }

    if (no->qtd_eventos > 0) {
        printf("  Apostas:");
        for (i = 0; i < no->qtd_eventos; i++) {
            printf("%s %s (D%d) %s", i > 0 ? " |" : "",
                   no->eventos[i].nome, no->eventos[i].dupla + 1,
                   texto_evento(no->eventos[i].tipo));
            if (no->eventos[i].tipo == CORREU)
                correu = 1;
        }
        printf("\n");
    }

    if (correu)
        printf("  Resultado: rodada interrompida, alguem correu\n");
    else if (no->rodada.dupla_vencedora == RODADA_EMPATADA)
        printf("  Resultado: empate\n");
    else
        printf("  Resultado: dupla %d levou\n", no->rodada.dupla_vencedora + 1);
}

/* Lê uma linha e devolve o primeiro caractere que não é espaço
 * (ou '\0' se a linha estiver vazia, EOF se a entrada acabou). */
static int ler_opcao(void)
{
    char linha[32];
    int i;

    if (fgets(linha, sizeof linha, stdin) == NULL)
        return EOF;
    if (strchr(linha, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; /* descarta o resto da linha comprida */
    }
    for (i = 0; linha[i] == ' ' || linha[i] == '\t'; i++)
        ;
    return (linha[i] == '\n') ? '\0' : linha[i];
}

void navegar_var(const Historico *h)
{
    const NoHist *atual;
    int posicao, opcao;

    if (h == NULL || h->qtd == 0) {
        printf("\nO VAR ainda esta vazio.\n");
        return;
    }

    atual = h->fim; /* começa pela rodada mais recente */
    posicao = h->qtd;

    for (;;) {
        exibir_no(atual, posicao, h->qtd);
        printf("[a]vancar  [v]oltar  [s]air: ");
        fflush(stdout);

        opcao = ler_opcao();
        if (opcao == EOF || opcao == 's' || opcao == 'S')
            return;

        if (opcao == 'a' || opcao == 'A') {
            if (atual->prox == NULL) {
                printf("Ja esta na ultima rodada.\n");
            } else {
                atual = atual->prox;
                posicao++;
            }
        } else if (opcao == 'v' || opcao == 'V') {
            if (atual->ant == NULL) {
                printf("Ja esta na primeira rodada.\n");
            } else {
                atual = atual->ant;
                posicao--;
            }
        } else {
            printf("Opcao invalida. Use a, v ou s.\n");
        }
    }
}

void liberar_historico(Historico *h)
{
    NoHist *no, *prox;

    if (h == NULL)
        return;
    for (no = h->inicio; no != NULL; no = prox) {
        prox = no->prox;
        free(no);
    }
    free(h);
}
