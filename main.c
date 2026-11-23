/*
 * main.c — Pacote 5: Integração (menu do bar e partida até 12)
 * Responsável: Arthur
 *
 * 3ª unidade: "o bar fechando as contas".
 *   1. ao abrir, carrega a caderneta de fregueses (caderneta.txt) e
 *      entra no menu do bar: nova partida, caderneta A-Z, procurar
 *      freguês, mural, VAR de partidas antigas (historico.txt) e
 *      fechar o bar;
 *   2. nova partida (jogar_partida): cadastra os 4 jogadores ligando
 *      cada um à sua ficha na caderneta (rosto novo ou freguês da casa),
 *      conta a partida para os 4 e salva no disco. A partida ganha um
 *      número no historico.txt, e cada rodada do VAR é acrescentada
 *      no arquivo logo que acontece;
 *   3. laço da partida: mãos até uma dupla fazer 12. A cada mão:
 *      embaralha, distribui, revela a vira, vê se é mão normal, mão de
 *      onze ou mão de ferro (item criativo) e joga a mão (mao.c);
 *   4. fim de cada mão: soma os pontos, conta a mão ganha para os 2 da
 *      dupla, registra a mão mais cara, salva a caderneta, gira o "mão"
 *      e oferece o VAR;
 *   5. fim da partida: anuncia a dupla vencedora, mostra o mural, o
 *      mural da vergonha com a mão mais cara e oferece o VAR;
 *   6. fechar o bar: mostra o mural, salva, libera tudo e sai.
 *
 * O jogo da mão em si (rodadas, truco, VAR) está em mao.c.
 *
 * Para compilar (todos os arquivos estão nesta mesma pasta):
 *     gcc -std=c99 -Wall -Wextra *.c -o bar_do_pato
 * Para executar:
 *     ./bar_do_pato        (no Windows: bar_do_pato.exe)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "baralho.h"
#include "jogador.h"
#include "manilha.h"
#include "fila_turno.h"
#include "rodada.h"
#include "truco.h"
#include "historico.h"
#include "caderneta.h"
#include "arquivo.h"
#include "mural.h"
#include "mao.h"

#define PONTOS_PARA_VENCER 12
#define PONTOS_MAO_DE_ONZE 11
#define TAM_LINHA          64

/* ------------------------------------------------------------------ */
/* Entrada                                                             */
/* ------------------------------------------------------------------ */

/* Lê um nome do teclado, sem o '\n' do fim e sem espaços nas pontas.
 * Se a linha for maior que o buffer, descarta o resto dela.
 * Retorna 1 se leu uma linha, 0 se a entrada acabou (EOF). */
static int ler_nome(const char *rotulo, char *destino)
{
    size_t n, ini;
    int ch;

    printf("%s: ", rotulo);
    fflush(stdout);
    if (fgets(destino, MAX_NOME, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }
    n = strlen(destino);
    if (n > 0 && destino[n - 1] == '\n')
        destino[--n] = '\0';
    else
        while ((ch = getchar()) != '\n' && ch != EOF)
            ; /* nome longo demais: joga fora o que sobrou da linha */

    /* tira espaços (e o '\r' do Windows) do fim e do começo */
    while (n > 0 && (destino[n - 1] == ' ' || destino[n - 1] == '\t' ||
                     destino[n - 1] == '\r'))
        destino[--n] = '\0';
    for (ini = 0; destino[ini] == ' ' || destino[ini] == '\t'; ini++)
        ;
    if (ini > 0)
        memmove(destino, destino + ini, n - ini + 1);
    return 1;
}

/* Mostra a mensagem e espera o usuário apertar ENTER (ou o fim da entrada). */
static void esperar_enter(const char *mensagem)
{
    int ch;

    printf("\nPressione ENTER para %s...", mensagem);
    fflush(stdout);
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
    printf("\n");
}

/* Pergunta se quer ver o VAR. Só "s" abre; ENTER ou EOF seguem o jogo. */
static int quer_ver_var(void)
{
    char linha[TAM_LINHA];
    int ch;

    printf("\nVer o VAR? [s] sim / ENTER continua: ");
    fflush(stdout);
    if (fgets(linha, sizeof linha, stdin) == NULL)
        return 0;
    if (strchr(linha, '\n') == NULL)
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    return linha[0] == 's' || linha[0] == 'S';
}

/* ------------------------------------------------------------------ */
/* Exibição                                                            */
/* ------------------------------------------------------------------ */

static void mostrar_placar(const Mesa *m)
{
    printf("  Placar: dupla 1 = %d | dupla 2 = %d\n",
           m->duplas[0]->pontos, m->duplas[1]->pontos);
}

/* Mostra a ordem de jogo desta mão, percorrendo a roda uma volta
 * completa: cada jogador tem a sua vez e a vez volta ao "mão". */
static void exibir_roda(const Mesa *m, FilaTurno *f)
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

/* Mostra a ficha completa de um freguês (opção 3 do menu). */
static void mostrar_ficha(const Fregues *f)
{
    double taxa = taxa_correu(f);

    printf("\nFicha de %s\n", f->nome);
    printf("  Partidas:          %d\n", f->partidas);
    printf("  Maos ganhas:       %d\n", f->maos_ganhas);
    printf("  Trucos pedidos:    %d\n", f->trucos_pedidos);
    printf("  Trucos recebidos:  %d\n", f->trucos_recebidos);
    printf("  Correu:            %d\n", f->correu);
    if (taxa < 0)
        printf("  Taxa de correu:    --\n");
    else
        printf("  Taxa de correu:    %.1f%%\n", taxa * 100.0);
}

/* ------------------------------------------------------------------ */
/* Distribuição                                                        */
/* ------------------------------------------------------------------ */

/* Dá 3 cartas a cada jogador, uma por vez, seguindo a roda de jogo:
 * começa pelo "mão" e segue a ordem da mesa. Toda carta sai do topo
 * do baralho. As mãos da distribuição anterior são descartadas antes. */
static void distribuir_cartas(Mesa *m, FilaTurno *f, Baralho *b)
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

/* ------------------------------------------------------------------ */
/* Mão de onze (item criativo)                                         */
/* ------------------------------------------------------------------ */

/* Olha o placar: as duas com 11 = ferro, so uma = onze, senao normal. */
static TipoMao tipo_da_mao(const Mesa *m)
{
    int onze0 = m->duplas[0]->pontos == PONTOS_MAO_DE_ONZE;
    int onze1 = m->duplas[1]->pontos == PONTOS_MAO_DE_ONZE;

    if (onze0 && onze1)
        return MAO_DE_FERRO;
    if (onze0 || onze1)
        return MAO_DE_ONZE;
    return MAO_NORMAL;
}

/* Mostra as cartas dos dois parceiros da dupla (0 ou 1), a vira e a
 * manilha, e pergunta [1] jogar valendo 3 / [2] correr.
 * Retorna 1 se vai jogar, 0 se correu. EOF = joga. */
static int decidir_mao_de_onze(const Mesa *m, int dupla, Carta vira,
                               Valor manilha)
{
    const Dupla *d = m->duplas[dupla];
    int i;

    printf("\n*** MAO DE ONZE para a dupla %d (%s e %s) ***\n", dupla + 1,
           d->jogadores[0]->nome, d->jogadores[1]->nome);
    printf("Os dois parceiros olham as cartas antes de decidir.\n");
    printf("  Vira: ");
    mostrar_carta(vira);
    printf(" | Manilha: %s\n", texto_valor(manilha));
    for (i = 0; i < 2; i++) {
        printf("\nCartas de %s:\n", d->jogadores[i]->nome);
        exibir_mao(d->jogadores[i]);
    }
    printf("\nDupla %d, o que voces fazem?\n", dupla + 1);
    printf("  [1] jogar (a mao vale 3)\n");
    printf("  [2] correr (a dupla %d leva 1 ponto)\n", 2 - dupla);

    return ler_opcao(1, 2) != 2; /* -1 (EOF) também joga */
}

/* A dupla com 11 correu da mão de onze: a mão vai para o VAR (memória
 * e disco) como uma rodada vazia, com o evento CORREU no nome do 1º
 * jogador da dupla. Não conta como "correu do truco" na caderneta. */
static void registrar_corrida_de_onze(VarPartida *var, const Mesa *m,
                                      int dupla, int num_mao)
{
    Rodada r;
    EventoAposta ev;

    memset(&r, 0, sizeof r);          /* qtd = 0: nenhuma carta jogada */
    r.dupla_vencedora = 1 - dupla;
    ev = criar_evento(dupla, m->duplas[dupla]->jogadores[0]->nome, CORREU);
    registrar_no_var(var, num_mao, 1, &r, &ev, 1);
}

/* ------------------------------------------------------------------ */
/* Cadastro                                                            */
/* ------------------------------------------------------------------ */

/* 1 se o nome já está sentado em jogadores[0..qtd-1] (sem diferenciar
 * maiúsculas, com comparar_nomes). */
static int ja_sentado(const Jogador jogadores[], int qtd, const char *nome)
{
    int i;

    for (i = 0; i < qtd; i++)
        if (comparar_nomes(jogadores[i].nome, nome) == 0)
            return 1;
    return 0;
}

/* Lê os 4 nomes (recusando vazio, com ';' e repetido na mesa), liga
 * cada jogador à sua ficha e conta a partida para ele.
 * Retorna 1 se cadastrou os 4, 0 se a entrada acabou ou faltou memória. */
static int cadastrar_jogadores(Jogador jogadores[], Caderneta *c)
{
    char nome[MAX_NOME];
    char rotulo[48];
    Fregues *ficha;
    int i, novo;

    for (i = 0; i < NUM_JOGADORES; i++) {
        snprintf(rotulo, sizeof rotulo, "Dupla %d - jogador %d",
                 i / 2 + 1, i % 2 + 1);

        for (;;) {
            if (!ler_nome(rotulo, nome))
                return 0; /* a entrada acabou */
            if (nome[0] == '\0')
                printf("O nome nao pode ficar vazio.\n");
            else if (strchr(nome, ';') != NULL)
                printf("O nome nao pode ter ';'.\n");
            else if (ja_sentado(jogadores, i, nome))
                printf("%s ja esta sentado nesta mesa.\n", nome);
            else
                break;
        }

        ficha = buscar_ou_cadastrar(c, nome, &novo);
        if (ficha == NULL) {
            printf("Erro: memoria insuficiente.\n");
            return 0;
        }
        if (novo)
            printf("Rosto novo: %s\n", ficha->nome);
        else
            printf("Fregues da casa: %s (%d partida%s, %d mao%s ganha%s)\n",
                   ficha->nome, ficha->partidas,
                   ficha->partidas == 1 ? "" : "s", ficha->maos_ganhas,
                   ficha->maos_ganhas == 1 ? "" : "s",
                   ficha->maos_ganhas == 1 ? "" : "s");

        /* a partida conta assim que ele senta (mesmo se for abandonada).
         * O jogador usa o nome como está na caderneta (o da 1ª vez). */
        ficha->partidas++;
        cadastrar_jogador(&jogadores[i], ficha->nome);
        jogadores[i].ficha = ficha;
    }
    return 1;
}

/* Salva a caderneta e avisa se não conseguiu (o jogo segue). */
static void salvar_ou_avisar(const Caderneta *c)
{
    if (!salvar_caderneta(c, ARQUIVO_CADERNETA))
        printf("Atencao: nao consegui salvar a caderneta.\n");
}

/* ------------------------------------------------------------------ */
/* Partida                                                             */
/* ------------------------------------------------------------------ */

/* Uma partida completa, do cadastro ao fim de jogo. Baralho, roda e VAR
 * são criados aqui e liberados no fim; a caderneta é do bar inteiro.
 * Retorna 0 se a entrada acabou no cadastro, senão 1. */
static int jogar_partida(Caderneta *c)
{
    Jogador jogadores[NUM_JOGADORES];
    Dupla duplas[2];
    Mesa mesa;
    Baralho *baralho;     /* pilha de cartas (tipo opaco) */
    FilaTurno *fila;      /* roda de jogo (tipo opaco) */
    Historico *historico; /* VAR: lista duplamente encadeada (tipo opaco) */
    VarPartida var;       /* o VAR + o número da partida no historico.txt */
    MaoMaisCara mais_cara;
    Carta vira;
    Valor manilha;
    TipoMao tipo;
    int i, num_mao, vencedor, pontos, campea, dupla_onze;

    printf("\n=== Nova partida ===\n");

    /* 1. cadastrar os 4 jogadores (jogadores 0 e 1 = dupla 1;
     *    2 e 3 = dupla 2), ligados às fichas da caderneta */
    if (!cadastrar_jogadores(jogadores, c)) {
        salvar_ou_avisar(c); /* quem já sentou tem a partida contada */
        return 0;
    }
    salvar_ou_avisar(c);

    /* 2. formar as duplas e montar a mesa com os parceiros alternados */
    criar_dupla(&duplas[0], &jogadores[0], &jogadores[1]);
    criar_dupla(&duplas[1], &jogadores[2], &jogadores[3]);
    montar_mesa(&mesa, &duplas[0], &duplas[1]);

    printf("\nMesa montada (parceiros alternados):\n");
    for (i = 0; i < NUM_JOGADORES; i++)
        printf("  Cadeira %d: %s (dupla %d)\n", i + 1,
               mesa.posicoes[i]->nome, numero_da_dupla(&mesa, mesa.posicoes[i]));

    /* 3. baralho, roda e VAR valem a partida inteira */
    baralho = criar_baralho();
    fila = criar_fila(&mesa, 0);
    historico = criar_historico();
    if (baralho == NULL || fila == NULL || historico == NULL) {
        printf("Erro: memoria insuficiente.\n");
        liberar_baralho(baralho);
        liberar_fila(fila);
        liberar_historico(historico);
        return 1;
    }
    iniciar_mao_mais_cara(&mais_cara);

    /* o VAR desta partida também vai para o historico.txt, com um
     * número novo: primeiro a linha P (data e os 4 nomes) */
    var.h = historico;
    var.id = proximo_id_partida(ARQUIVO_HISTORICO);
    var.avisou = 0;
    if (!gravar_inicio_partida(ARQUIVO_HISTORICO, var.id, &mesa))
        avisar_falha_var(&var);

    /* 4. laço da partida: mãos até uma dupla fazer 12 */
    num_mao = 1;
    while (mesa.duplas[0]->pontos < PONTOS_PARA_VENCER &&
           mesa.duplas[1]->pontos < PONTOS_PARA_VENCER) {
        printf("\n========== Mao %d ==========\n", num_mao);
        exibir_roda(&mesa, fila);

        embaralhar(baralho);
        distribuir_cartas(&mesa, fila, baralho);
        vira = revelar_vira(baralho);
        manilha = calcular_manilha(vira);

        printf("  Vira: ");
        mostrar_carta(vira);
        printf(" | Manilha: %s\n", texto_valor(manilha));
        mostrar_placar(&mesa);

        tipo = tipo_da_mao(&mesa);
        if (tipo == MAO_DE_ONZE) {
            dupla_onze = (mesa.duplas[0]->pontos == PONTOS_MAO_DE_ONZE) ? 0 : 1;
            if (decidir_mao_de_onze(&mesa, dupla_onze, vira, manilha)) {
                vencedor = jogar_mao(&mesa, fila, manilha, &var, num_mao,
                                     tipo, &pontos);
            } else {
                vencedor = 1 - dupla_onze;
                pontos = 1;
                printf("\nA dupla %d correu da mao de onze.\n", dupla_onze + 1);
                registrar_corrida_de_onze(&var, &mesa, dupla_onze, num_mao);
            }
        } else {
            vencedor = jogar_mao(&mesa, fila, manilha, &var, num_mao,
                                 tipo, &pontos);
        }

        /* fim da mão, nesta ordem: pontos, mãos ganhas, mão mais cara,
         * salvar, girar o "mão" e VAR */
        if (vencedor != -1) {
            Dupla *d = mesa.duplas[vencedor];

            d->pontos += pontos;
            printf("\n>> Dupla %d levou a mao %d (+%d).\n", vencedor + 1,
                   num_mao, pontos);
            for (i = 0; i < 2; i++)
                if (d->jogadores[i]->ficha != NULL)
                    d->jogadores[i]->ficha->maos_ganhas++;
            registrar_mao(&mais_cara, num_mao, pontos, d, vencedor + 1);
        }
        mostrar_placar(&mesa);
        salvar_ou_avisar(c);

        /* quem era "mão" vai para o fim da roda; o próximo assume */
        girar_mao(fila);

        if (mesa.duplas[0]->pontos < PONTOS_PARA_VENCER &&
            mesa.duplas[1]->pontos < PONTOS_PARA_VENCER && quer_ver_var())
            navegar_var(historico);
        num_mao++;
    }

    /* 5. fim da partida */
    campea = (mesa.duplas[0]->pontos >= PONTOS_PARA_VENCER) ? 0 : 1;
    printf("\n========== Fim de jogo ==========\n");
    printf("Dupla %d venceu: %s e %s!\n", campea + 1,
           mesa.duplas[campea]->jogadores[0]->nome,
           mesa.duplas[campea]->jogadores[1]->nome);
    mostrar_placar(&mesa);
    printf("Rodadas no VAR: %d\n", tamanho_historico(historico));

    /* fecha a partida no historico.txt (sem a linha F ela aparece
     * como "interrompida" na lista do menu) */
    if (!gravar_fim_partida(ARQUIVO_HISTORICO, var.id, mesa.duplas[0]->pontos,
                            mesa.duplas[1]->pontos, campea + 1))
        avisar_falha_var(&var);

    mostrar_mural(c);
    mostrar_mural_vergonha(c, &mais_cara);

    if (quer_ver_var())
        navegar_var(historico);

    liberar_baralho(baralho);
    liberar_fila(fila);
    liberar_historico(historico);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Menu do bar                                                         */
/* ------------------------------------------------------------------ */

/* Opção 3: lê um nome e mostra a ficha, ou avisa que nunca sentou. */
static void procurar_fregues(const Caderneta *c)
{
    char nome[MAX_NOME];
    const Fregues *f;

    if (!ler_nome("Nome do fregues", nome) || nome[0] == '\0')
        return;
    f = buscar_fregues(c, nome);
    if (f == NULL)
        printf("Esse ai nunca sentou aqui.\n");
    else
        mostrar_ficha(f);
}

/* Opção 5: lista as partidas do historico.txt e abre o VAR da escolhida.
 * O número digitado é o número da partida (0 volta ao menu). */
static void ver_partidas_antigas(void)
{
    Historico *h;
    int total, escolha;

    total = listar_partidas_salvas(ARQUIVO_HISTORICO);
    if (total == 0) {
        printf("\nNenhuma partida salva.\n");
        return;
    }

    /* o maior número pode passar do total se alguma linha P se perdeu */
    printf("\nQual partida rever? (0 volta)\n");
    escolha = ler_opcao(0, proximo_id_partida(ARQUIVO_HISTORICO) - 1);
    if (escolha <= 0)
        return;

    h = carregar_partida(ARQUIVO_HISTORICO, escolha);
    if (h == NULL) {
        printf("Nao achei a partida %d.\n", escolha);
        return;
    }
    printf("\nVAR da partida %d (%d rodada%s)\n", escolha,
           tamanho_historico(h), tamanho_historico(h) == 1 ? "" : "s");
    navegar_var(h);
    liberar_historico(h);
}

int main(void)
{
    Caderneta caderneta;
    int opcao, carregados;

    srand((unsigned) time(NULL)); /* única semente do programa */

    printf("=== Truco no Bar do Pato ===\n\n");

    iniciar_caderneta(&caderneta);
    carregados = carregar_caderneta(&caderneta, ARQUIVO_CADERNETA);
    printf("Caderneta carregada: %d fregues%s\n", carregados,
           carregados == 1 ? "" : "es");

    do {
        printf("\n=== Bar do Pato ===\n");
        printf("[1] Nova partida\n");
        printf("[2] Ver a caderneta (A-Z)\n");
        printf("[3] Procurar fregues\n");
        printf("[4] Mural do bar\n");
        printf("[5] VAR de partidas antigas\n");
        printf("[0] Fechar o bar\n");

        opcao = ler_opcao(0, 5);
        switch (opcao) {
        case 1:
            if (!jogar_partida(&caderneta))
                opcao = 0; /* a entrada acabou no meio do cadastro */
            break;
        case 2:
            printf("\n");
            listar_caderneta(&caderneta);
            break;
        case 3:
            procurar_fregues(&caderneta);
            break;
        case 4:
            mostrar_mural(&caderneta);
            mostrar_mural_vergonha(&caderneta, NULL);
            break;
        case 5:
            ver_partidas_antigas();
            break;
        default:
            opcao = 0; /* 0 ou EOF: fecha o bar */
            break;
        }
    } while (opcao != 0);

    /* fechar o bar: a noite termina com o mural */
    printf("\nFechando o bar...\n");
    mostrar_mural(&caderneta);
    salvar_ou_avisar(&caderneta);
    liberar_caderneta(&caderneta);

    /* segura a janela aberta quando o .exe é aberto com dois cliques */
    esperar_enter("sair");
    return 0;
}
