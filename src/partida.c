/*
 * partida.c — 3ª unidade, Pacote 6 (versão 2): Motor da partida
 * Responsável: Arthur
 *
 * Mesmas regras e mesma ordem do jogar_mao / resolver_pedido / laço da
 * partida da versão 1 (mao.c e main.c), só que quebradas em ações:
 *
 *   versão 1 (espera o teclado)        versão 2 (motor)
 *   ---------------------------        ------------------------------
 *   escolher_carta + jogar_carta       acao_jogar_carta
 *   "[2] pedir truco"                  acao_pedir
 *   laço do resolver_pedido            acao_responder (uma volta)
 *   decidir_mao_de_onze                acao_decidir_onze
 *   fim da rodada / fim da mão         acao_continuar
 *
 * Nenhuma função daqui lê o teclado ou escreve na tela: o que
 * aconteceu vai para p->aviso, e as falhas de disco para
 * p->falhou_caderneta / p->falhou_var.
 */
#include <stdio.h>
#include <string.h>

#include "partida.h"
#include "arquivo.h"
#include "baralho.h"
#include "manilha.h"
#include "rodada.h"

/* ------------------------------------------------------------------ */
/* Pequenas ajudas                                                     */
/* ------------------------------------------------------------------ */

int dupla_do_jogador(const Partida *p, const Jogador *j)
{
    return (j != NULL && j->dupla == p->mesa.duplas[1]) ? 1 : 0;
}

/* Quem senta logo depois de j: sempre um adversário (a mesa alterna as
 * duplas). É quem responde o pedido de j. */
static Jogador *vizinho(const Partida *p, const Jogador *j)
{
    int i;

    for (i = 0; i < NUM_JOGADORES; i++)
        if (p->mesa.posicoes[i] == j)
            return p->mesa.posicoes[(i + 1) % NUM_JOGADORES];
    return NULL;
}

/* Anota um evento de aposta da rodada atual (ignora se o vetor encheu). */
static void anotar(Partida *p, const Jogador *j, TipoEvento tipo)
{
    if (p->qtd_eventos < MAX_EVENTOS) {
        p->eventos[p->qtd_eventos] =
            criar_evento(dupla_do_jogador(p, j), j->nome, tipo);
        p->qtd_eventos++;
    }
}

/* Pedido degrau -> evento do VAR (1 = truco, 2 = seis, 3 = doze). */
static TipoEvento evento_do_nivel(int nivel)
{
    return (TipoEvento) (PEDIU_TRUCO + nivel - 1);
}

/* Registra a rodada no VAR da memória e logo depois acrescenta no
 * historico.txt. Falha de gravação só liga p->falhou_var (o jogo segue). */
static void registrar_no_var(Partida *p, const Rodada *r)
{
    if (!registrar_rodada(p->historico, p->num_mao, p->num_rodada, r,
                          p->eventos, p->qtd_eventos) ||
        !gravar_rodada(ARQUIVO_HISTORICO, p->id_partida,
                       ultima_rodada(p->historico)))
        p->falhou_var = 1;
}

static void salvar(Partida *p)
{
    p->falhou_caderneta = !salvar_caderneta(p->caderneta, ARQUIVO_CADERNETA);
}

/* Olha o placar: as duas com 11 = ferro, só uma = onze, senão normal. */
static TipoMao tipo_da_mao(const Partida *p)
{
    int onze0 = p->duplas[0].pontos == PONTOS_MAO_DE_ONZE;
    int onze1 = p->duplas[1].pontos == PONTOS_MAO_DE_ONZE;

    if (onze0 && onze1)
        return MAO_DE_FERRO;
    if (onze0 || onze1)
        return MAO_DE_ONZE;
    return MAO_NORMAL;
}

/* Dá 3 cartas a cada jogador, uma por vez, seguindo a roda a partir do
 * "mão" (igual ao distribuir_cartas da versão 1). */
static void distribuir_cartas(Partida *p)
{
    int i, c;

    for (i = 0; i < NUM_JOGADORES; i++)
        p->jogadores[i].qtd_cartas = 0;

    for (c = 0; c < CARTAS_NA_MAO; c++) {
        for (i = 0; i < NUM_JOGADORES; i++) {
            Jogador *j = proximo_jogador(p->fila);
            if (puxar_do_topo(p->baralho, &j->mao[j->qtd_cartas]))
                j->qtd_cartas++;
            jogador_passou(p->fila);
        }
    }
}

/* Mesa limpa para a rodada p->num_rodada: sem cartas, sem eventos, e a
 * roda girada até quem abre. */
static void abrir_rodada(Partida *p)
{
    memset(&p->rodada, 0, sizeof p->rodada);
    p->qtd_eventos = 0;
    p->idx_vencedora = -1;
    fila_posicionar(p->fila, p->abre);
    p->estado = EST_VEZ_DE_JOGAR;
}

/* Embaralha, distribui, revela a vira e vê o tipo da mão. */
static void comecar_mao(Partida *p)
{
    embaralhar(p->baralho);
    distribuir_cartas(p);
    p->vira = revelar_vira(p->baralho);
    p->manilha = calcular_manilha(p->vira);
    p->tipo = tipo_da_mao(p);
    p->num_rodada = 1;
    p->correram = 0;
    p->dono_da_mao = -1;
    p->pontos_da_mao = 0;
    p->pediu = p->responde = NULL;
    p->abre = proximo_jogador(p->fila); /* o "mão" abre a 1ª rodada */
    abrir_rodada(p);

    if (p->tipo == MAO_DE_ONZE) {
        iniciar_aposta(&p->aposta);   /* bloqueia ao decidir jogar */
        p->dupla_onze = (p->duplas[0].pontos == PONTOS_MAO_DE_ONZE) ? 0 : 1;
        p->estado = EST_DECIDIR_ONZE;
        snprintf(p->aviso, sizeof p->aviso,
                 "Mão de onze para a Dupla %d: jogar ou correr?",
                 p->dupla_onze + 1);
    } else if (p->tipo == MAO_DE_FERRO) {
        iniciar_aposta_bloqueada(&p->aposta, 1);
        snprintf(p->aviso, sizeof p->aviso,
                 "MÃO DE FERRO! Sem truco: quem levar, ganha a partida.");
    } else {
        iniciar_aposta(&p->aposta);
        snprintf(p->aviso, sizeof p->aviso, "Mão %d: vez de %s.",
                 p->num_mao, p->abre->nome);
    }
}

/* Fim da mão, na ordem da versão 1: pontos, mãos ganhas, mão mais cara,
 * salvar, girar o "mão". dono = 0/1, ou -1 se ninguém pontua. */
static void fechar_mao(Partida *p, int dono, int pontos)
{
    int i;

    p->dono_da_mao = dono;
    p->pontos_da_mao = (dono == -1) ? 0 : pontos;
    if (dono != -1) {
        Dupla *d = &p->duplas[dono];

        d->pontos += pontos;
        for (i = 0; i < 2; i++)
            if (d->jogadores[i]->ficha != NULL)
                d->jogadores[i]->ficha->maos_ganhas++;
        registrar_mao(&p->mais_cara, p->num_mao, pontos, d, dono + 1);
    }
    salvar(p);
    girar_mao(p->fila);
    p->estado = EST_FIM_MAO;
}

/* ------------------------------------------------------------------ */
/* Cadastro e início                                                   */
/* ------------------------------------------------------------------ */

const char *problema_no_nome(const char *nome, const char *aceitos[], int n)
{
    const char *c;
    int i, so_espaco = 1;

    if (nome == NULL)
        return "O nome não pode ficar vazio.";
    for (c = nome; *c != '\0'; c++)
        if (*c != ' ' && *c != '\t')
            so_espaco = 0;
    if (so_espaco)
        return "O nome não pode ficar vazio.";
    if (strchr(nome, ';') != NULL)
        return "O nome não pode ter ';'.";
    if (strlen(nome) >= MAX_NOME)
        return "Nome comprido demais.";
    for (i = 0; i < n; i++)
        if (aceitos[i] != NULL && comparar_nomes(aceitos[i], nome) == 0)
            return "Esse nome já está sentado nesta mesa.";
    return NULL;
}

int iniciar_partida(Partida *p, const char *nomes[NUM_JOGADORES],
                    Caderneta *c)
{
    int i, novo;

    memset(p, 0, sizeof *p);
    p->caderneta = c;

    p->baralho = criar_baralho();
    p->historico = criar_historico();
    if (p->baralho == NULL || p->historico == NULL) {
        encerrar_partida(p);
        return 0;
    }

    /* jogadores 0 e 1 = dupla 1; 2 e 3 = dupla 2. A partida conta assim
     * que ele senta, e o jogador usa o nome como está na caderneta. */
    for (i = 0; i < NUM_JOGADORES; i++) {
        Fregues *ficha = buscar_ou_cadastrar(c, nomes[i], &novo);

        if (ficha == NULL) {
            encerrar_partida(p);
            return 0;
        }
        ficha->partidas++;
        cadastrar_jogador(&p->jogadores[i], ficha->nome);
        p->jogadores[i].ficha = ficha;
    }

    criar_dupla(&p->duplas[0], &p->jogadores[0], &p->jogadores[1]);
    criar_dupla(&p->duplas[1], &p->jogadores[2], &p->jogadores[3]);
    montar_mesa(&p->mesa, &p->duplas[0], &p->duplas[1]);

    p->fila = criar_fila(&p->mesa, 0);
    if (p->fila == NULL) {
        encerrar_partida(p);
        return 0;
    }
    salvar(p);

    iniciar_mao_mais_cara(&p->mais_cara);
    p->id_partida = proximo_id_partida(ARQUIVO_HISTORICO);
    if (!gravar_inicio_partida(ARQUIVO_HISTORICO, p->id_partida, &p->mesa))
        p->falhou_var = 1;

    p->num_mao = 1;
    comecar_mao(p);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Consultas                                                           */
/* ------------------------------------------------------------------ */

Jogador *jogador_da_vez(const Partida *p)
{
    switch (p->estado) {
    case EST_DECIDIR_ONZE:
        return p->duplas[p->dupla_onze].jogadores[0];
    case EST_VEZ_DE_JOGAR:
        return proximo_jogador(p->fila);
    case EST_RESPONDER_PEDIDO:
        return p->responde;
    default:
        return NULL;
    }
}

int pode_pedir_agora(const Partida *p)
{
    if (p->estado != EST_VEZ_DE_JOGAR)
        return 0;
    return pode_pedir(&p->aposta, dupla_do_jogador(p, jogador_da_vez(p)));
}

/* ------------------------------------------------------------------ */
/* Ações                                                               */
/* ------------------------------------------------------------------ */

int acao_jogar_carta(Partida *p, int idx)
{
    Jogador *j, *venceu;
    int i;

    if (p->estado != EST_VEZ_DE_JOGAR)
        return 0;
    j = jogador_da_vez(p);
    if (j == NULL || idx < 0 || idx >= j->qtd_cartas ||
        p->rodada.qtd >= NUM_JOGADORES)
        return 0;

    jogar_carta(j, idx, &p->rodada.cartas[p->rodada.qtd]);
    p->rodada.quem[p->rodada.qtd] = j;
    p->rodada.qtd++;
    jogador_passou(p->fila);

    if (p->rodada.qtd < NUM_JOGADORES) {
        snprintf(p->aviso, sizeof p->aviso, "%s jogou. Vez de %s.", j->nome,
                 proximo_jogador(p->fila)->nome);
        return 1;
    }

    /* 4ª carta: quem levou, VAR e se a mão já tem dono */
    apurar_rodada(&p->rodada, p->manilha, &p->mesa);
    registrar_no_var(p, &p->rodada);
    p->resultados[p->num_rodada - 1] = p->rodada.dupla_vencedora;
    p->situacao_mao = apurar_mao(p->resultados, p->num_rodada);

    /* quem venceu abre a próxima; se empatou, abre quem abriu esta */
    venceu = vencedor_da_rodada(&p->rodada, p->manilha);
    p->idx_vencedora = -1;
    if (venceu != NULL) {
        p->abre = venceu;
        for (i = 0; i < p->rodada.qtd; i++)
            if (p->rodada.quem[i] == venceu)
                p->idx_vencedora = i;
    }

    if (p->rodada.dupla_vencedora == RODADA_EMPATADA)
        snprintf(p->aviso, sizeof p->aviso, "Rodada %d empatou.",
                 p->num_rodada);
    else
        snprintf(p->aviso, sizeof p->aviso, "Dupla %d levou a rodada %d.",
                 p->rodada.dupla_vencedora + 1, p->num_rodada);
    p->estado = EST_FIM_RODADA;
    return 1;
}

int acao_pedir(Partida *p)
{
    Jogador *j;

    if (!pode_pedir_agora(p))
        return 0;
    j = jogador_da_vez(p);

    pedir(&p->aposta, dupla_do_jogador(p, j));
    if (j->ficha != NULL)
        j->ficha->trucos_pedidos++;
    anotar(p, j, evento_do_nivel(p->aposta.nivel));

    p->pediu = j;
    p->responde = vizinho(p, j);
    snprintf(p->aviso, sizeof p->aviso, "%s pediu %s!", j->nome,
             nome_nivel(p->aposta.nivel));
    p->estado = EST_RESPONDER_PEDIDO;
    return 1;
}

int acao_responder(Partida *p, Resposta r)
{
    Jogador *resp, *troca;
    int pontos;

    if (p->estado != EST_RESPONDER_PEDIDO || p->responde == NULL)
        return 0;
    if (r != ACEITAR && r != CORRER && r != AUMENTAR)
        return 0;
    if (r == AUMENTAR && p->aposta.nivel >= NIVEL_MAXIMO)
        return 0;
    resp = p->responde;

    /* estatística: ele respondeu um pedido */
    if (resp->ficha != NULL)
        resp->ficha->trucos_recebidos++;

    if (r == ACEITAR) {
        responder(&p->aposta, ACEITAR);
        anotar(p, resp, ACEITOU);
        snprintf(p->aviso, sizeof p->aviso, "%s aceitou. A mão vale %d.",
                 resp->nome, p->aposta.valor);
        p->pediu = p->responde = NULL;
        p->estado = EST_VEZ_DE_JOGAR; /* o mesmo jogador segue na vez */
    } else if (r == CORRER) {
        pontos = responder(&p->aposta, CORRER);
        anotar(p, resp, CORREU);
        if (resp->ficha != NULL)
            resp->ficha->correu++;
        snprintf(p->aviso, sizeof p->aviso, "%s correu! Dupla %d leva %d.",
                 resp->nome, p->aposta.dupla_pediu + 1, pontos);
        /* a mão acaba aqui e a rodada parcial vai para o VAR */
        p->rodada.dupla_vencedora = p->aposta.dupla_pediu;
        registrar_no_var(p, &p->rodada);
        p->correram = 1;
        p->pediu = p->responde = NULL;
        fechar_mao(p, p->aposta.dupla_pediu, pontos);
    } else {
        responder(&p->aposta, AUMENTAR);
        anotar(p, resp, evento_do_nivel(p->aposta.nivel));
        /* aumentar também é um pedido feito por quem aumentou */
        if (resp->ficha != NULL)
            resp->ficha->trucos_pedidos++;
        snprintf(p->aviso, sizeof p->aviso, "%s pediu %s!", resp->nome,
                 nome_nivel(p->aposta.nivel));
        /* quem aumentou vira quem pediu; responde o outro lado */
        troca = p->pediu;
        p->pediu = resp;
        p->responde = troca;
    }
    return 1;
}

int acao_decidir_onze(Partida *p, int jogar)
{
    Rodada vazia;
    const Dupla *d;

    if (p->estado != EST_DECIDIR_ONZE)
        return 0;
    d = &p->duplas[p->dupla_onze];

    if (jogar) {
        iniciar_aposta_bloqueada(&p->aposta, 3);
        snprintf(p->aviso, sizeof p->aviso,
                 "MÃO DE ONZE! Vale 3 e ninguém pode pedir truco.");
        p->estado = EST_VEZ_DE_JOGAR;
        return 1;
    }

    /* correu: a mão vai para o VAR como uma rodada vazia, com o evento
     * CORREU no nome do 1º jogador da dupla. Não conta como "correu do
     * truco" na caderneta. */
    memset(&vazia, 0, sizeof vazia);
    vazia.dupla_vencedora = 1 - p->dupla_onze;
    p->qtd_eventos = 0;
    anotar(p, d->jogadores[0], CORREU);
    registrar_no_var(p, &vazia);
    p->correram = 1;
    snprintf(p->aviso, sizeof p->aviso,
             "A Dupla %d correu da mão de onze. Dupla %d leva 1.",
             p->dupla_onze + 1, 2 - p->dupla_onze);
    fechar_mao(p, 1 - p->dupla_onze, 1);
    return 1;
}

int acao_continuar(Partida *p)
{
    if (p->estado == EST_FIM_RODADA) {
        if (p->situacao_mao == MAO_EM_ABERTO &&
            p->num_rodada < RODADAS_POR_MAO) {
            p->num_rodada++;
            abrir_rodada(p);
            snprintf(p->aviso, sizeof p->aviso, "Rodada %d: %s abre.",
                     p->num_rodada, p->abre->nome);
        } else if (p->situacao_mao == MAO_EMPATADA) {
            fechar_mao(p, -1, 0);
            snprintf(p->aviso, sizeof p->aviso,
                     "As 3 rodadas empataram: ninguém pontua.");
        } else {
            fechar_mao(p, p->situacao_mao, p->aposta.valor);
            snprintf(p->aviso, sizeof p->aviso, "+%d para a Dupla %d.",
                     p->pontos_da_mao, p->dono_da_mao + 1);
        }
        return 1;
    }

    if (p->estado == EST_FIM_MAO) {
        if (p->duplas[0].pontos >= PONTOS_PARA_VENCER ||
            p->duplas[1].pontos >= PONTOS_PARA_VENCER) {
            p->campea = (p->duplas[0].pontos >= PONTOS_PARA_VENCER) ? 0 : 1;
            if (!gravar_fim_partida(ARQUIVO_HISTORICO, p->id_partida,
                                    p->duplas[0].pontos, p->duplas[1].pontos,
                                    p->campea + 1))
                p->falhou_var = 1;
            snprintf(p->aviso, sizeof p->aviso, "Dupla %d venceu: %s e %s!",
                     p->campea + 1, p->duplas[p->campea].jogadores[0]->nome,
                     p->duplas[p->campea].jogadores[1]->nome);
            p->estado = EST_FIM_PARTIDA;
        } else {
            p->num_mao++;
            comecar_mao(p);
        }
        return 1;
    }
    return 0;
}

void encerrar_partida(Partida *p)
{
    liberar_baralho(p->baralho);
    liberar_fila(p->fila);
    liberar_historico(p->historico);
    p->baralho = NULL;
    p->fila = NULL;
    p->historico = NULL;
}
