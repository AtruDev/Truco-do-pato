/*
 * main.c — Módulo 5: Fluxo principal e exibição
 * Responsável: Arthur
 *
 * Integra os módulos da 1ª unidade:
 *   1. cadastra os 4 jogadores e forma as 2 duplas;
 *   2. monta a mesa com os parceiros alternados (D1, D2, D1, D2);
 *   3. cria o baralho (pilha) e a roda de jogo (fila) uma vez só;
 *   4. a cada mão: embaralha o baralho, distribui 3 cartas para cada
 *      jogador (sempre pelo topo), revela a vira e calcula a manilha;
 *   5. mostra a roda de jogo (quem joga em cada vez) e gira o "mão"
 *      para a próxima mão: quem era "mão" vai para o fim da roda.
 *
 * Nesta unidade ninguém joga carta ainda: as cartas são apenas
 * distribuídas e mostradas na tela.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "baralho.h"
#include "jogador.h"
#include "manilha.h"
#include "fila_turno.h"

/* Quantas mãos são mostradas na demonstração. Com 4 mãos cada jogador
 * é "mão" uma vez; a 5ª mostra que a roda volta ao primeiro. */
#define MAOS_DEMONSTRACAO 5

/* Nomes para exibição, na mesma ordem dos enums Valor e Naipe. */
const char *NOMES_VALOR[NUM_VALORES] = {
    "4", "5", "6", "7", "Q", "J", "K", "A", "2", "3"
};

const char *NOMES_NAIPE[NUM_NAIPES] = {
    "ouros", "espadas", "copas", "paus"
};

/* imprime uma carta no formato "3 de paus" */
void imprimir_carta(Carta c)
{
    printf("%s de %s", NOMES_VALOR[c.valor], NOMES_NAIPE[c.naipe]);
}

/* lê um nome do teclado, sem o '\n' do fim.
 * Se a linha for maior que o buffer, descarta o resto dela. */
void ler_nome(const char *rotulo, char *destino)
{
    size_t n;
    int ch;

    printf("%s: ", rotulo);
    if (fgets(destino, MAX_NOME, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }
    n = strlen(destino);
    if (n > 0 && destino[n - 1] == '\n')
        destino[n - 1] = '\0';
    else
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; /* nome longo demais: joga fora o que sobrou da linha */
}

/* mostra a mensagem e espera o usuário apertar ENTER (ou o fim da entrada) */
void esperar_enter(const char *mensagem)
{
    int ch;

    printf("\nPressione ENTER para %s...", mensagem);
    fflush(stdout);
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
    printf("\n");
}

/* número (1 ou 2) da dupla do jogador, para exibição */
int numero_da_dupla(const Mesa *m, const Jogador *j)
{
    return (j->dupla == m->duplas[0]) ? 1 : 2;
}

/* Dá 3 cartas a cada jogador, uma por vez, seguindo a roda de jogo:
 * começa pelo "mão" e segue a ordem da mesa. Toda carta sai do topo
 * do baralho. As mãos da distribuição anterior são descartadas antes. */
void distribuir_cartas(Mesa *m, FilaTurno *f, Baralho *b)
{
    int i, c;

    for (i = 0; i < NUM_JOGADORES; i++)
        m->posicoes[i]->qtd_cartas = 0;

    for (c = 0; c < CARTAS_NA_MAO; c++) {
        for (i = 0; i < NUM_JOGADORES; i++) {
            Jogador *j = proximo_jogador(f);
            if (puxar_do_topo(b, &j->mao[j->qtd_cartas]))
                j->qtd_cartas++;
            jogador_passou(f); /* a vez passa para o próximo da roda */
        }
    }
}

/* Mostra a ordem de jogo desta mão, percorrendo a roda uma volta
 * completa: cada jogador tem a sua vez e a vez volta ao "mão". */
void exibir_roda(const Mesa *m, FilaTurno *f)
{
    int vez = 1;
    int voltou = 0;

    printf("Ordem de jogo: ");
    while (!voltou) {
        Jogador *j = proximo_jogador(f);
        printf("%s%s (D%d)", vez > 1 ? " -> " : "", j->nome,
               numero_da_dupla(m, j));
        if (vez == 1)
            printf(" [mao]");
        voltou = jogador_passou(f);
        vez++;
    }
    printf("\n");
}

/* imprime as cartas de cada jogador (na ordem da roda), vira e manilha */
void exibir_estado(const Mesa *m, FilaTurno *f, Carta vira, Valor manilha)
{
    int i, c;

    printf("\n");
    for (i = 0; i < NUM_JOGADORES; i++) {
        const Jogador *j = proximo_jogador(f);
        printf("  %d) %-15s (dupla %d) | ", i + 1, j->nome,
               numero_da_dupla(m, j));
        for (c = 0; c < j->qtd_cartas; c++) {
            imprimir_carta(j->mao[c]);
            printf(c + 1 < j->qtd_cartas ? ", " : "");
        }
        printf("\n");
        jogador_passou(f);
    }

    printf("\n  Vira: ");
    imprimir_carta(vira);
    printf("\n  Manilha: %s (os quatro %s)\n", NOMES_VALOR[manilha],
           NOMES_VALOR[manilha]);
    printf("  Pontos: dupla 1 = %d | dupla 2 = %d\n",
           m->duplas[0]->pontos, m->duplas[1]->pontos);
}

int main() {
    Jogador jogadores[NUM_JOGADORES];
    Dupla duplas[2];
    Mesa mesa;
    Baralho *baralho;   /* pilha de cartas (tipo opaco) */
    FilaTurno *fila;    /* roda de jogo (tipo opaco) */
    Carta vira;
    Valor manilha;
    char nome[MAX_NOME];
    int i, num_mao;

    srand((unsigned) time(NULL)); /* única semente do programa */

    printf("=== Truco no Bar do Pato ===\n\n");

    /* 1. ler os nomes e cadastrar os 4 jogadores.
     *    Jogadores 0 e 1 formam a dupla 1; jogadores 2 e 3, a dupla 2. */
    for (i = 0; i < NUM_JOGADORES; i++) {
        char rotulo[48];
        sprintf(rotulo, "Dupla %d - jogador %d", i / 2 + 1, i % 2 + 1);
        ler_nome(rotulo, nome);
        if (nome[0] == '\0')
            sprintf(nome, "Jogador %d", i + 1);
        cadastrar_jogador(&jogadores[i], nome);
    }

    /* 2. formar as duplas e montar a mesa com os parceiros alternados */
    criar_dupla(&duplas[0], &jogadores[0], &jogadores[1]);
    criar_dupla(&duplas[1], &jogadores[2], &jogadores[3]);
    montar_mesa(&mesa, &duplas[0], &duplas[1]);

    printf("\nMesa montada (parceiros alternados):\n");
    for (i = 0; i < NUM_JOGADORES; i++)
        printf("  Cadeira %d: %s (dupla %d)\n", i + 1,
               mesa.posicoes[i]->nome, numero_da_dupla(&mesa, mesa.posicoes[i]));

    /* 3. baralho e roda são criados uma vez só e reaproveitados em todas
     *    as mãos; a roda começa na cadeira 1 */
    baralho = criar_baralho();
    fila = criar_fila(&mesa, 0);
    if (baralho == NULL || fila == NULL) {
        printf("Erro: memoria insuficiente.\n");
        liberar_baralho(baralho);
        liberar_fila(fila);
        return 1;
    }

    /* 4. uma sequência de mãos para mostrar a roda girando */
    for (num_mao = 1; num_mao <= MAOS_DEMONSTRACAO; num_mao++) {
        printf("\n========== Mao %d ==========\n", num_mao);
        exibir_roda(&mesa, fila);

        /* as 40 cartas voltam e são embaralhadas a cada mão */
        embaralhar(baralho);

        /* 12 cartas saem do topo para os jogadores, depois a vira */
        distribuir_cartas(&mesa, fila, baralho);
        vira = revelar_vira(baralho);
        manilha = calcular_manilha(vira);

        exibir_estado(&mesa, fila, vira, manilha);

        /* quem era "mão" vai para o fim da roda; o próximo assume */
        girar_mao(fila);

        if (num_mao < MAOS_DEMONSTRACAO)
            esperar_enter("a proxima mao");
    }

    liberar_baralho(baralho);
    liberar_fila(fila);

    printf("\nFim da demonstracao da 1a unidade.\n");
    /* segura a janela aberta quando o .exe é aberto com dois cliques */
    esperar_enter("sair");
    return 0;
}
