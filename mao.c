/*
 * mao.c — 3ª unidade, Pacote 5 (recomendado): Módulo da mão
 * Responsável: Arthur
 *
 * jogar_mao, resolver_pedido, vizinho, anotar, evento_do_nivel e
 * exibir_cartas_na_mesa vieram do main.c da 2ª entrega. O que mudou:
 *   - jogar_mao recebe o TipoMao e inicia a aposta bloqueada na mão de
 *     onze (vale 3) e na mão de ferro (vale 1), com um aviso na tela;
 *   - pedido feito  -> ficha->trucos_pedidos++ de quem pediu;
 *     pedido respondido -> ficha->trucos_recebidos++ de quem respondeu;
 *     correu -> ficha->correu++; aumentou -> ficha->trucos_pedidos++;
 *   - as funções que só este módulo usa passaram a ser static;
 *   - os 2 registrar_rodada (rodada normal e corrida) viraram
 *     registrar_no_var, que também grava a rodada no historico.txt.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mao.h"
#include "arquivo.h"
#include "caderneta.h"
#include "rodada.h"
#include "truco.h"

#define TAM_LINHA 64

/* ------------------------------------------------------------------ */
/* Entrada e exibição (públicas, o main também usa)                    */
/* ------------------------------------------------------------------ */

int ler_opcao(int min, int max)
{
    char linha[TAM_LINHA];
    char *fim;
    long valor;
    size_t n;
    int ch;

    for (;;) {
        printf("Opcao (%d a %d): ", min, max);
        fflush(stdout);
        if (fgets(linha, sizeof linha, stdin) == NULL)
            return -1;

        n = strlen(linha);
        if (n > 0 && linha[n - 1] == '\n') {
            linha[n - 1] = '\0';
        } else if (n == sizeof linha - 1) {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("Entrada muito longa.\n");
            continue;
        }

        valor = strtol(linha, &fim, 10);
        if (fim == linha) {
            printf("Entrada invalida. Digite um numero de %d a %d.\n", min, max);
            continue;
        }
        while (*fim == ' ' || *fim == '\t' || *fim == '\r')
            fim++;
        if (*fim != '\0' || valor < min || valor > max) {
            printf("Entrada invalida. Digite um numero de %d a %d.\n", min, max);
            continue;
        }
        return (int) valor;
    }
}

int numero_da_dupla(const Mesa *m, const Jogador *j)
{
    return (j->dupla == m->duplas[0]) ? 1 : 2;
}

int indice_da_dupla(const Mesa *m, const Jogador *j)
{
    return numero_da_dupla(m, j) - 1;
}

void exibir_cartas_na_mesa(const Mesa *m, const Rodada *r)
{
    int i;

    if (r->qtd == 0) {
        printf("  Mesa: (vazia)\n");
        return;
    }
    printf("  Mesa:");
    for (i = 0; i < r->qtd; i++) {
        printf("%s %s (D%d) ", i > 0 ? " |" : "", r->quem[i]->nome,
               numero_da_dupla(m, r->quem[i]));
        mostrar_carta(r->cartas[i]);
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/* VAR na memória e no disco                                           */
/* ------------------------------------------------------------------ */

void avisar_falha_var(VarPartida *var)
{
    if (!var->avisou) {
        printf("Atencao: nao consegui gravar o VAR.\n");
        var->avisou = 1;
    }
}

int registrar_no_var(VarPartida *var, int mao, int rod, const Rodada *r,
                     const EventoAposta ev[], int n)
{
    /* se registrar_rodada falhar, ultima_rodada seria a rodada anterior:
     * por isso só grava quando o registro deu certo */
    if (!registrar_rodada(var->h, mao, rod, r, ev, n) ||
        !gravar_rodada(ARQUIVO_HISTORICO, var->id, ultima_rodada(var->h))) {
        avisar_falha_var(var);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Truco                                                               */
/* ------------------------------------------------------------------ */

/* Quem senta logo depois de j. Como a mesa alterna as duplas, é sempre
 * um adversário: é ele quem responde o pedido de j. */
static Jogador *vizinho(const Mesa *m, const Jogador *j)
{
    int i;

    for (i = 0; i < NUM_JOGADORES; i++)
        if (m->posicoes[i] == j)
            return m->posicoes[(i + 1) % NUM_JOGADORES];
    return NULL;
}

/* Anota um evento de aposta da rodada (ignora se o vetor encheu). */
static void anotar(EventoAposta ev[], int *qtd, const Mesa *m,
                   const Jogador *j, TipoEvento tipo)
{
    if (*qtd < MAX_EVENTOS) {
        ev[*qtd] = criar_evento(indice_da_dupla(m, j), j->nome, tipo);
        (*qtd)++;
    }
}

/* Pedido degrau -> evento do VAR (1 = truco, 2 = seis, 3 = doze). */
static TipoEvento evento_do_nivel(int nivel)
{
    return (TipoEvento) (PEDIU_TRUCO + nivel - 1);
}

/* O jogador da vez pediu: a resposta vai para a dupla adversária e,
 * a cada aumento, volta para a outra. Termina quando alguém aceita ou
 * corre. Retorna os pontos de quem pediu se alguém correu, senão 0.
 * Soma recebidos/correu/pedidos nas fichas de quem responde. */
static int resolver_pedido(const Mesa *m, Aposta *a, Jogador *quem_pediu,
                           EventoAposta ev[], int *qtd_ev)
{
    static const char *const VALE[] = { "1", "3", "6", "12" };
    Jogador *pediu = quem_pediu;
    Jogador *responde = vizinho(m, quem_pediu);
    Jogador *troca;
    int opcao, max, pontos;

    while (a->aguardando) {
        printf("\n%s (D%d) pediu %s! Se aceitar, a mao vale %s.\n",
               pediu->nome, numero_da_dupla(m, pediu), nome_nivel(a->nivel),
               VALE[a->nivel]);
        printf("%s (D%d), o que voce faz?\n", responde->nome,
               numero_da_dupla(m, responde));
        printf("  [1] aceitar\n  [2] correr\n");
        max = 2;
        if (a->nivel < NIVEL_MAXIMO) {
            printf("  [3] pedir %s\n", nome_nivel(a->nivel + 1));
            max = 3;
        }

        opcao = ler_opcao(1, max);
        if (opcao == -1)
            opcao = 1; /* entrada acabou: aceita */

        /* estatística: ele respondeu um pedido */
        if (responde->ficha != NULL)
            responde->ficha->trucos_recebidos++;

        if (opcao == 1) {
            responder(a, ACEITAR);
            anotar(ev, qtd_ev, m, responde, ACEITOU);
            printf("%s aceitou. A mao vale %d.\n", responde->nome, a->valor);
        } else if (opcao == 2) {
            pontos = responder(a, CORRER);
            anotar(ev, qtd_ev, m, responde, CORREU);
            if (responde->ficha != NULL)
                responde->ficha->correu++;
            printf("%s correu! Dupla %d leva %d ponto(s).\n", responde->nome,
                   a->dupla_pediu + 1, pontos);
            return pontos;
        } else {
            responder(a, AUMENTAR);
            anotar(ev, qtd_ev, m, responde, evento_do_nivel(a->nivel));
            /* aumentar também é um pedido feito por quem aumentou */
            if (responde->ficha != NULL)
                responde->ficha->trucos_pedidos++;
            /* quem aumentou vira quem pediu; responde o outro lado */
            troca = pediu;
            pediu = responde;
            responde = troca;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Mão                                                                 */
/* ------------------------------------------------------------------ */

int jogar_mao(Mesa *m, FilaTurno *f, Valor manilha, VarPartida *var,
              int num_mao, TipoMao tipo, int *pontos)
{
    Aposta aposta;
    int resultados[RODADAS_POR_MAO];
    int num_rodada, vez, dono = MAO_EM_ABERTO;
    Jogador *abre = proximo_jogador(f); /* o "mão" abre a 1ª rodada */

    if (tipo == MAO_DE_ONZE) {
        iniciar_aposta_bloqueada(&aposta, 3);
        printf("\nMAO DE ONZE! Ninguem pode pedir truco nesta mao.\n");
    } else if (tipo == MAO_DE_FERRO) {
        iniciar_aposta_bloqueada(&aposta, 1);
        printf("\nMAO DE FERRO! Ninguem pode pedir truco nesta mao."
               " Quem levar, ganha a partida.\n");
    } else {
        iniciar_aposta(&aposta);
    }

    for (num_rodada = 1; num_rodada <= RODADAS_POR_MAO && dono == MAO_EM_ABERTO;
         num_rodada++) {
        Rodada r;
        EventoAposta ev[MAX_EVENTOS];
        int qtd_ev = 0;
        Jogador *venceu;

        memset(&r, 0, sizeof r);
        fila_posicionar(f, abre);
        printf("\n---------- Mao %d, rodada %d ----------\n", num_mao, num_rodada);

        for (vez = 0; vez < NUM_JOGADORES; vez++) {
            Jogador *j = proximo_jogador(f);
            int dupla = indice_da_dupla(m, j);
            int idx = -1;

            while (idx == -1) {
                printf("\nVez de %s (D%d) | a mao vale %d\n", j->nome,
                       numero_da_dupla(m, j), aposta.valor);
                exibir_cartas_na_mesa(m, &r);

                /* na mão de onze/ferro pode_pedir é sempre 0:
                 * a opção de truco nem aparece */
                if (pode_pedir(&aposta, dupla)) {
                    printf("  [1] jogar carta\n  [2] pedir %s\n",
                           nome_nivel(aposta.nivel + 1));
                    if (ler_opcao(1, 2) == 2) {
                        int corrida;

                        pedir(&aposta, dupla);
                        if (j->ficha != NULL)
                            j->ficha->trucos_pedidos++;
                        anotar(ev, &qtd_ev, m, j, evento_do_nivel(aposta.nivel));
                        corrida = resolver_pedido(m, &aposta, j, ev, &qtd_ev);
                        if (corrida > 0) {
                            /* alguém correu: a mão acaba aqui e a rodada
                             * parcial vai para o VAR */
                            r.dupla_vencedora = aposta.dupla_pediu;
                            registrar_no_var(var, num_mao, num_rodada, &r,
                                             ev, qtd_ev);
                            *pontos = corrida;
                            return aposta.dupla_pediu;
                        }
                        continue; /* aceitou: o mesmo jogador segue na vez */
                    }
                }

                idx = escolher_carta(j);
                if (idx == -1)
                    idx = 0; /* entrada acabou: joga a primeira carta */
            }

            jogar_carta(j, idx, &r.cartas[r.qtd]);
            r.quem[r.qtd] = j;
            r.qtd++;
            jogador_passou(f);
        }

        /* fim da rodada: quem levou, VAR e se a mão já tem dono */
        printf("\n");
        exibir_cartas_na_mesa(m, &r);
        apurar_rodada(&r, manilha, m);
        if (r.dupla_vencedora == RODADA_EMPATADA)
            printf(">> Rodada %d empatou.\n", num_rodada);
        else
            printf(">> Rodada %d: dupla %d levou.\n", num_rodada,
                   r.dupla_vencedora + 1);

        registrar_no_var(var, num_mao, num_rodada, &r, ev, qtd_ev);
        resultados[num_rodada - 1] = r.dupla_vencedora;
        dono = apurar_mao(resultados, num_rodada);

        /* quem venceu abre a próxima; se empatou, abre quem abriu esta */
        venceu = vencedor_da_rodada(&r, manilha);
        if (venceu != NULL)
            abre = venceu;
    }

    if (dono == MAO_EMPATADA) {
        printf("\nAs 3 rodadas empataram: ninguem pontua.\n");
        *pontos = 0;
        return -1;
    }

    *pontos = aposta.valor;
    return dono;
}
