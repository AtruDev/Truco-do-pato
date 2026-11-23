/*
 * gui_mesa.c — 3ª unidade, Pacote 7: a mesa (a partida)
 * Responsável: Arthur
 *
 * Assentos fixos, parceiros frente a frente como no bar: cadeira 1
 * embaixo, 2 à esquerda, 3 em cima, 4 à direita (D1 = 1 e 3, D2 = 2 e
 * 4). O assento da vez brilha em amarelo.
 *
 * Privacidade: todos olham a mesma tela, então as mãos ficam viradas.
 * Na vez de alguém aparece a cortina "Vez da Bia — clique para ver
 * suas cartas", e só então as 3 cartas dela aparecem na bandeja de
 * baixo. No pedido de truco, quem responde também tem a sua cortina;
 * na mão de onze, a dupla abre as 6 cartas.
 *
 * Teclado: 1–3 cartas, T truco, Espaço continuar/abrir a cortina,
 * V VAR, Esc menu. No pedido: 1 aceitar, 2 correr, 3 aumentar.
 * Na mão de onze: 1 jogar valendo 3, 2 correr.
 *
 * Tudo o que muda a partida passa pelas acao_* do motor (partida.h).
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"

#define ALTURA_MESA     120.0f  /* cartas jogadas no centro */
#define ALTURA_BANDEJA  150.0f  /* cartas da mão aberta */
#define ALTURA_PEQUENA   46.0f  /* verso das mãos fechadas */
#define Y_BANDEJA       592.0f
#define ESPACO_BANDEJA  132.0f

/* ------------------------------------------------------------------ */
/* Geometria (a mesma para o atualizar e o desenhar)                   */
/* ------------------------------------------------------------------ */

/* Assento (0..3, ordem da mesa) do jogador j. */
static int assento_de(const Partida *p, const Jogador *j)
{
    int i;

    for (i = 0; i < NUM_JOGADORES; i++)
        if (p->mesa.posicoes[i] == j)
            return i;
    return 0;
}

/* Centro da plaquinha com o nome de cada assento. */
static Vector2 centro_assento(int s)
{
    static const Vector2 c[NUM_JOGADORES] = {
        { 640, 700 }, { 110, 300 }, { 640, 98 }, { 1170, 300 }
    };
    return c[s];
}

/* Onde a carta jogada por cada assento para na mesa. */
static Vector2 lugar_na_mesa(int s)
{
    static const Vector2 c[NUM_JOGADORES] = {
        { 640, 415 }, { 528, 315 }, { 640, 215 }, { 752, 315 }
    };
    return c[s];
}

static Rectangle carta_da_bandeja(int i, int n)
{
    float x0 = 640.0f - (n - 1) * ESPACO_BANDEJA / 2.0f;
    return retangulo_carta(x0 + i * ESPACO_BANDEJA, Y_BANDEJA, ALTURA_BANDEJA);
}

static Rectangle cortina(void)   { return (Rectangle){ 390, 528, 500, 146 }; }
static Rectangle bot_truco(void) { return (Rectangle){ 1060, 548, 196, 96 }; }
static Rectangle bot_var(void)   { return (Rectangle){ 1050, 12, 96, 42 }; }
static Rectangle bot_menu(void)  { return (Rectangle){ 1158, 12, 106, 42 }; }
static Rectangle bot_continuar(void) { return (Rectangle){ 500, 584, 280, 60 }; }

/* Botões do pedido (0 aceitar, 1 correr, 2 aumentar). */
static Rectangle bot_resposta(int i)
{
    return (Rectangle){ 330.0f + i * 216.0f, 490, 200, 60 };
}

/* Carta do jogador que responde, dentro da janela do pedido. */
static Rectangle carta_resposta(int i)
{
    return retangulo_carta(560.0f + i * 80.0f, 398, 104);
}

static Rectangle bot_onze(int jogar)
{
    return jogar ? (Rectangle){ 330, 562, 300, 60 } : (Rectangle){ 650, 562, 300, 60 };
}

static Rectangle carta_onze(int parceiro, int i)
{
    return retangulo_carta(470.0f + i * 84.0f + parceiro * 340.0f, 410, 112);
}

static Rectangle janela_modal(void) { return (Rectangle){ 250, 150, 780, 500 }; }

/* ------------------------------------------------------------------ */
/* Atualizar                                                           */
/* ------------------------------------------------------------------ */

static int espaco(void)
{
    return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
           IsKeyPressed(KEY_KP_ENTER);
}

/* Índice da carta da bandeja escolhida (clique ou tecla 1–3), ou -1. */
static int carta_escolhida(const Jogador *j)
{
    int i;

    for (i = 0; i < j->qtd_cartas; i++) {
        if (IsKeyPressed(KEY_ONE + i) || IsKeyPressed(KEY_KP_1 + i))
            return i;
        if (botao_clicado(carta_da_bandeja(i, j->qtd_cartas), 1))
            return i;
    }
    return -1;
}

void tela_mesa_abrir(Contexto *ctx)
{
    EstadoMesa *m = &ctx->mesa;
    int i;

    /* voltar do VAR/menu fecha as cortinas de novo (privacidade) */
    m->revelado = NULL;
    m->resposta_revelada = NULL;
    m->onze_revelado = 0;
    if (ctx->cadastro.foco == -1 || m->de_qual == NULL ||
        m->id_partida != ctx->partida.id_partida) {
        /* partida nova: zera animações, placar e avisos */
        memset(m, 0, sizeof *m);
        m->de_qual = &ctx->partida;
        m->id_partida = ctx->partida.id_partida;
        m->ultimo_estado = ctx->partida.estado;
        for (i = 0; i < 2; i++)
            m->placar_mostrado[i] = (float) ctx->partida.duplas[i].pontos;
        ctx->cadastro.foco = 0;
    }
}

static void jogar(Contexto *ctx, int idx)
{
    Partida *p = &ctx->partida;
    EstadoMesa *m = &ctx->mesa;
    Jogador *j = jogador_da_vez(p);
    Rectangle de = carta_da_bandeja(idx, j->qtd_cartas);
    Carta c = j->mao[idx];

    if (acao_jogar_carta(p, idx)) {
        m->anim_ativa = 1;
        m->anim_carta = c;
        m->anim_de = (Vector2){ de.x + de.width / 2, de.y + de.height / 2 };
        m->anim_para = lugar_na_mesa(assento_de(p, j));
        m->anim_inicio = GetTime();
        tocar(&ctx->rec, ctx->rec.som_carta);
    }
}

Tela tela_mesa_atualizar(Contexto *ctx)
{
    Partida *p = &ctx->partida;
    EstadoMesa *m = &ctx->mesa;
    Jogador *j;
    float dt = GetFrameTime();
    int i, idx;

    if (!ctx->partida_ativa)
        return TELA_MENU;
    if (p->estado == EST_FIM_PARTIDA)
        return TELA_FIM_PARTIDA;

    /* coisas que acontecem quando o estado muda */
    if (p->num_mao != m->ultima_mao) {
        m->ultima_mao = p->num_mao;
        m->onze_revelado = 0;
        tocar(&ctx->rec, ctx->rec.som_embaralhar);
    }
    if (p->estado == EST_RESPONDER_PEDIDO &&
        (m->ultimo_estado != EST_RESPONDER_PEDIDO || p->responde != m->ultimo_responde)) {
        m->pedido_inicio = GetTime();
        m->resposta_revelada = NULL;
        tocar(&ctx->rec, ctx->rec.som_truco);
    }
    if ((p->estado == EST_FIM_RODADA || p->estado == EST_FIM_MAO) &&
        m->ultimo_estado != p->estado)
        m->faixa_inicio = GetTime();
    m->ultimo_estado = p->estado;
    m->ultimo_responde = p->responde;

    if ((p->falhou_var || p->falhou_caderneta) && !m->aviso_disco) {
        m->aviso_disco = 1;
        m->aviso_disco_inicio = GetTime();
    }

    /* o placar sobe devagar até o de verdade */
    for (i = 0; i < 2; i++) {
        float alvo = (float) p->duplas[i].pontos;
        m->placar_mostrado[i] += (alvo - m->placar_mostrado[i]) * fminf(1.0f, dt * 5.0f);
        if (fabsf(alvo - m->placar_mostrado[i]) < 0.02f)
            m->placar_mostrado[i] = alvo;
    }

    /* barra do topo */
    if (botao_clicado(bot_var(), 1) || IsKeyPressed(KEY_V)) {
        ctx->var_aberto = p->historico;
        ctx->var.var_proprio = 0;
        ctx->var.id_antigo = 0;
        return TELA_VAR;
    }
    if (botao_clicado(bot_menu(), 1) || IsKeyPressed(KEY_ESCAPE))
        return TELA_MENU;

    j = jogador_da_vez(p);
    switch (p->estado) {
    case EST_DECIDIR_ONZE:
        if (!m->onze_revelado) {
            if (espaco() || botao_clicado(janela_modal(), 1))
                m->onze_revelado = 1;
        } else if (botao_clicado(bot_onze(1), 1) || IsKeyPressed(KEY_ONE) ||
                   IsKeyPressed(KEY_KP_1)) {
            acao_decidir_onze(p, 1);
        } else if (botao_clicado(bot_onze(0), 1) || IsKeyPressed(KEY_TWO) ||
                   IsKeyPressed(KEY_KP_2)) {
            acao_decidir_onze(p, 0);
        }
        break;

    case EST_VEZ_DE_JOGAR:
        if (m->revelado != j) {
            if (espaco() || botao_clicado(cortina(), 1))
                m->revelado = j;
            break;
        }
        if ((botao_clicado(bot_truco(), pode_pedir_agora(p)) || IsKeyPressed(KEY_T)) &&
            acao_pedir(p))
            break;
        idx = carta_escolhida(j);
        if (idx >= 0)
            jogar(ctx, idx);
        break;

    case EST_RESPONDER_PEDIDO:
        if (m->resposta_revelada != j && espaco())
            m->resposta_revelada = j;
        for (i = 0; i < 3 && m->resposta_revelada != j; i++)
            if (botao_clicado(carta_resposta(i), 1))
                m->resposta_revelada = j;
        if (botao_clicado(bot_resposta(0), 1) || IsKeyPressed(KEY_ONE) ||
            IsKeyPressed(KEY_KP_1))
            acao_responder(p, ACEITAR);
        else if (botao_clicado(bot_resposta(1), 1) || IsKeyPressed(KEY_TWO) ||
                 IsKeyPressed(KEY_KP_2))
            acao_responder(p, CORRER);
        else if (p->aposta.nivel < NIVEL_MAXIMO &&
                 (botao_clicado(bot_resposta(2), 1) || IsKeyPressed(KEY_THREE) ||
                  IsKeyPressed(KEY_KP_3)))
            acao_responder(p, AUMENTAR);
        break;

    case EST_FIM_RODADA:
    case EST_FIM_MAO:
        if (espaco() || botao_clicado(bot_continuar(), 1))
            acao_continuar(p);
        break;

    default:
        break;
    }

    if (p->estado == EST_FIM_PARTIDA) {
        tocar(&ctx->rec, ctx->rec.som_vitoria);
        return TELA_FIM_PARTIDA;
    }
    return TELA_MESA;
}

/* ------------------------------------------------------------------ */
/* Desenhar                                                            */
/* ------------------------------------------------------------------ */

static void desenhar_topo(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    char linha[256];
    int i;

    DrawRectangle(0, 0, LARGURA, 66, COR_MADEIRA);
    DrawRectangle(0, 66, LARGURA, 4, COR_PAINEL);

    for (i = 0; i < 2; i++) {
        float x = 20.0f + i * 420.0f;
        int mostrado = (int) floorf(ctx->mesa.placar_mostrado[i] + 0.5f);
        int animando = mostrado != p->duplas[i].pontos ||
                       ctx->mesa.placar_mostrado[i] != (float) p->duplas[i].pontos;

        snprintf(linha, sizeof linha, "Dupla %d", i + 1);
        texto(r, linha, x, 6, 18, COR_CREME_FRACO);
        snprintf(linha, sizeof linha, "%s & %s", p->duplas[i].jogadores[0]->nome,
                 p->duplas[i].jogadores[1]->nome);
        texto(r, linha, x, 30, TAM_TEXTO + 2, COR_CREME);
        snprintf(linha, sizeof linha, "%d", mostrado);
        texto_direita(r, linha, x + 380, 8, TAM_TITULO,
                      animando ? COR_PATO : COR_CREME);
    }

    snprintf(linha, sizeof linha, "Mão %d", p->num_mao);
    texto(r, linha, 870, 8, TAM_TEXTO, COR_CREME);
    snprintf(linha, sizeof linha, "vale %d", p->aposta.valor);
    DrawRectangleRounded((Rectangle){ 868, 34, 96, 26 }, 0.5f, 6,
                         p->aposta.valor > 1 ? COR_PERIGO : COR_PATO);
    texto_centro(r, linha, 916, 36, 19, p->aposta.valor > 1 ? COR_CREME : COR_PAINEL);
    if (p->tipo != MAO_NORMAL) {
        texto(r, p->tipo == MAO_DE_ONZE ? "onze" : "ferro", 975, 36, 19, COR_PATO);
    }
    desenhar_botao(r, bot_var(), "VAR", BOTAO_NORMAL, 1);
    desenhar_botao(r, bot_menu(), "Menu", BOTAO_NORMAL, 1);
}

static void desenhar_baralho_e_vira(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    Carta qualquer = { OUROS, QUATRO };
    char linha[48];
    int i;

    for (i = 3; i >= 0; i--)
        desenhar_carta(r, qualquer, retangulo_carta(250.0f - i * 1.5f, 175.0f - i * 1.5f,
                                                   106), 1, 0);
    desenhar_carta(r, p->vira, retangulo_carta(338, 175, 106), 0, 0);
    texto_centro(r, "vira", 338, 232, 18, COR_CREME_FRACO);
    snprintf(linha, sizeof linha, "Manilha: %s", texto_valor(p->manilha));
    texto_centro(r, linha, 294, 256, TAM_TEXTO + 2, COR_PATO);
}

static void desenhar_assentos(const Contexto *ctx, const Jogador *vez)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    int s, k;

    for (s = 0; s < NUM_JOGADORES; s++) {
        const Jogador *j = p->mesa.posicoes[s];
        Vector2 c = centro_assento(s);
        Rectangle placa = { c.x - 105, c.y - 20, 210, 40 };
        int da_vez = (j == vez);
        char linha[160];
        float bx, by;

        if (da_vez) {
            float pulso = 0.5f + 0.5f * sinf((float) GetTime() * 5.0f);
            DrawRectangleRounded((Rectangle){ placa.x - 6, placa.y - 6,
                                              placa.width + 12, placa.height + 12 },
                                 0.5f, 8, Fade(COR_PATO, 0.25f + 0.25f * pulso));
        }
        DrawRectangleRounded(placa, 0.5f, 8, da_vez ? COR_PATO : COR_PAINEL);
        snprintf(linha, sizeof linha, "%s · D%d", j->nome, dupla_do_jogador(p, j) + 1);
        texto_centro(r, linha, c.x, c.y - 11, TAM_TEXTO, da_vez ? COR_PAINEL : COR_CREME);

        /* a mão fechada: um verso por carta, ao lado da plaquinha */
        for (k = 0; k < j->qtd_cartas; k++) {
            Carta qualquer = { OUROS, QUATRO };
            if (s == 0 || s == 2) {
                bx = c.x + 135 + k * 24;
                by = c.y;
            } else {
                bx = c.x - 24 + k * 24;
                by = c.y + 58;
            }
            desenhar_carta(r, qualquer, retangulo_carta(bx, by, ALTURA_PEQUENA), 1, 0);
        }
    }
}

static void desenhar_cartas_na_mesa(const Contexto *ctx)
{
    const Partida *p = &ctx->partida;
    const EstadoMesa *m = &ctx->mesa;
    int i;

    for (i = 0; i < p->rodada.qtd; i++) {
        Vector2 c = lugar_na_mesa(assento_de(p, p->rodada.quem[i]));
        int destaque = p->estado == EST_FIM_RODADA && i == p->idx_vencedora;

        if (i == p->rodada.qtd - 1 && m->anim_ativa) {
            float t = progresso(m->anim_inicio, DURACAO_ANIM * 1.4f);
            float h = ALTURA_BANDEJA + (ALTURA_MESA - ALTURA_BANDEJA) * t;
            c.x = m->anim_de.x + (c.x - m->anim_de.x) * t;
            c.y = m->anim_de.y + (c.y - m->anim_de.y) * t;
            desenhar_carta(&ctx->rec, p->rodada.cartas[i], retangulo_carta(c.x, c.y, h),
                           0, destaque);
        } else {
            desenhar_carta(&ctx->rec, p->rodada.cartas[i],
                           retangulo_carta(c.x, c.y, ALTURA_MESA), 0, destaque);
        }
    }
}

static void desenhar_bandeja(const Contexto *ctx, const Jogador *j)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    char linha[256];
    int i;

    if (ctx->mesa.revelado != j) {
        Rectangle c = cortina();
        desenhar_painel(c);
        snprintf(linha, sizeof linha, "Vez de %s", j->nome);
        texto_centro(r, linha, c.x + c.width / 2, c.y + 24, 34, COR_PATO);
        texto_centro(r, "clique para ver suas cartas (Espaço)", c.x + c.width / 2,
                     c.y + 78, TAM_TEXTO, COR_CREME);
        texto_centro(r, "os outros, olhem para o lado!", c.x + c.width / 2, c.y + 108,
                     18, COR_CREME_FRACO);
        return;
    }

    for (i = 0; i < j->qtd_cartas; i++) {
        Rectangle rc = carta_da_bandeja(i, j->qtd_cartas);
        int sobre = mouse_em(rc);

        if (sobre)
            rc.y -= 12;  /* passar o mouse levanta a carta */
        desenhar_carta(r, j->mao[i], rc, 0, sobre);
        /* tecla da carta numa fichinha no canto de cima */
        snprintf(linha, sizeof linha, "%d", i + 1);
        DrawCircle((int) (rc.x + rc.width - 4), (int) (rc.y + 4), 13, COR_PAINEL);
        texto_centro(r, linha, rc.x + rc.width - 4, rc.y - 7, 18, COR_PATO);
    }
    (void) p;
}

static void desenhar_botao_truco(const Contexto *ctx, const Jogador *j)
{
    const Partida *p = &ctx->partida;
    static const char *const ROTULO[] = { "TRUCO!", "PEDIR SEIS", "PEDIR DOZE",
                                          "DOZE" };
    int pode = p->estado == EST_VEZ_DE_JOGAR && ctx->mesa.revelado == j &&
               pode_pedir_agora(p);
    int nivel = p->aposta.nivel < NIVEL_MAXIMO ? p->aposta.nivel : NIVEL_MAXIMO;

    desenhar_botao(&ctx->rec, bot_truco(), ROTULO[nivel], BOTAO_PERIGO, pode);
    if (pode)
        texto_centro(&ctx->rec, "(T)", bot_truco().x + bot_truco().width / 2,
                     bot_truco().y + bot_truco().height + 4, 18, COR_CREME_FRACO);
}

static void desenhar_pedido(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    const Jogador *resp = p->responde;
    static const char *const VALE[] = { "1", "3", "6", "12" };
    char grito[16], linha[128];
    float t = progresso(ctx->mesa.pedido_inicio, 0.35f);
    float tam = 40.0f + 56.0f * t;
    Rectangle jm = janela_modal();
    size_t k;
    int i;

    escurecer(0.55f);
    desenhar_painel(jm);

    snprintf(grito, sizeof grito, "%s!", nome_nivel(p->aposta.nivel));
    for (k = 0; grito[k] != '\0'; k++)
        if (grito[k] >= 'a' && grito[k] <= 'z')
            grito[k] = (char) (grito[k] - 'a' + 'A');
    texto_centro(r, grito, LARGURA / 2.0f, 230 - tam / 2, tam, COR_PERIGO);

    snprintf(linha, sizeof linha, "%s (Dupla %d) pediu %s. Se aceitar, a mão vale %s.",
             p->pediu->nome, dupla_do_jogador(p, p->pediu) + 1,
             nome_nivel(p->aposta.nivel), VALE[p->aposta.nivel]);
    texto_centro(r, linha, LARGURA / 2.0f, 280, TAM_TEXTO, COR_CREME);
    snprintf(linha, sizeof linha, "%s, o que você faz?", resp->nome);
    texto_centro(r, linha, LARGURA / 2.0f, 308, TAM_SUBTITULO, COR_PATO);

    /* as cartas de quem responde, com a cortina dele */
    for (i = 0; i < resp->qtd_cartas; i++)
        desenhar_carta(r, resp->mao[i], carta_resposta(i),
                       ctx->mesa.resposta_revelada != resp, 0);
    if (ctx->mesa.resposta_revelada != resp)
        texto_centro(r, "clique nas cartas (ou Espaço) para ver as suas",
                     LARGURA / 2.0f, 456, 18, COR_CREME_FRACO);

    desenhar_botao(r, bot_resposta(0), "[1] Aceitar", BOTAO_PRINCIPAL, 1);
    desenhar_botao(r, bot_resposta(1), "[2] Correr", BOTAO_NORMAL, 1);
    if (p->aposta.nivel < NIVEL_MAXIMO) {
        snprintf(linha, sizeof linha, "[3] Pedir %s", nome_nivel(p->aposta.nivel + 1));
        desenhar_botao(r, bot_resposta(2), linha, BOTAO_PERIGO, 1);
    }
    snprintf(linha, sizeof linha, "Correr agora dá %s ponto%s para a Dupla %d.",
             VALE[p->aposta.nivel - 1], p->aposta.nivel == 1 ? "" : "s",
             p->aposta.dupla_pediu + 1);
    texto_centro(r, linha, LARGURA / 2.0f, 574, 18, COR_CREME_FRACO);
}

static void desenhar_onze(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    const Dupla *d = &p->duplas[p->dupla_onze];
    Rectangle jm = janela_modal();
    char linha[256];
    int k, i;

    escurecer(0.6f);
    desenhar_painel(jm);
    texto_centro(r, "MÃO DE ONZE", LARGURA / 2.0f, jm.y + 20, TAM_TITULO, COR_PATO);
    snprintf(linha, sizeof linha, "Dupla %d (%s e %s): jogar valendo 3 ou correr?",
             p->dupla_onze + 1, d->jogadores[0]->nome, d->jogadores[1]->nome);
    texto_centro(r, linha, LARGURA / 2.0f, jm.y + 82, TAM_TEXTO + 2, COR_CREME);
    snprintf(linha, sizeof linha, "Vira: %s de %s  ·  Manilha: %s", texto_valor(p->vira.valor),
             texto_naipe(p->vira.naipe), texto_valor(p->manilha));
    texto_centro(r, linha, LARGURA / 2.0f, jm.y + 114, TAM_TEXTO, COR_CREME_FRACO);

    for (k = 0; k < 2; k++) {
        const Jogador *j = d->jogadores[k];
        Rectangle c0 = carta_onze(k, 1);
        texto_centro(r, j->nome, c0.x + c0.width / 2, 312, TAM_TEXTO, COR_PATO);
        for (i = 0; i < j->qtd_cartas; i++)
            desenhar_carta(r, j->mao[i], carta_onze(k, i), !ctx->mesa.onze_revelado, 0);
    }

    if (!ctx->mesa.onze_revelado) {
        snprintf(linha, sizeof linha,
                 "Só a Dupla %d olha: clique (ou Espaço) para ver as 6 cartas",
                 p->dupla_onze + 1);
        texto_centro(r, linha, LARGURA / 2.0f, 500, TAM_TEXTO, COR_CREME);
        return;
    }
    desenhar_botao(r, bot_onze(1), "[1] Jogar valendo 3", BOTAO_PRINCIPAL, 1);
    desenhar_botao(r, bot_onze(0), "[2] Correr (eles levam 1)", BOTAO_NORMAL, 1);
}

/* Faixa de fim de rodada / fim de mão, com o botão Continuar. */
static void desenhar_faixa(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    char linha[256];
    float t = progresso(ctx->mesa.faixa_inicio, DURACAO_ANIM);
    Color cor = COR_PATO;

    if (p->estado == EST_FIM_RODADA) {
        if (p->rodada.dupla_vencedora == RODADA_EMPATADA)
            snprintf(linha, sizeof linha, "Rodada %d empatou", p->num_rodada);
        else
            snprintf(linha, sizeof linha, "Dupla %d levou a rodada",
                     p->rodada.dupla_vencedora + 1);
    } else if (p->dono_da_mao == -1) {
        snprintf(linha, sizeof linha, "Ninguém pontuou nesta mão");
        cor = COR_CREME;
    } else {
        snprintf(linha, sizeof linha, "+%d para a Dupla %d", p->pontos_da_mao,
                 p->dono_da_mao + 1);
    }

    DrawRectangle(0, (int) (512 - 26 * t), LARGURA, (int) (52 * t),
                  (Color){ 0x2B, 0x18, 0x0D, 210 });
    if (t > 0.5f)
        texto_centro(r, linha, LARGURA / 2.0f, 494, 36, cor);
    if (p->estado == EST_FIM_MAO && p->correram)
        texto_centro(r, p->aviso, LARGURA / 2.0f, 548, TAM_TEXTO, COR_CREME);
    desenhar_botao(r, bot_continuar(), "Continuar (Espaço)", BOTAO_PRINCIPAL, 1);
}

void tela_mesa_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    const Jogador *vez;

    if (!ctx->partida_ativa)
        return;
    vez = jogador_da_vez(p);

    desenhar_feltro();
    DrawEllipse(640, 320, 330, 210, (Color){ 0, 0, 0, 40 });
    DrawEllipseLines(640, 320, 330, 210, (Color){ 0xF4, 0xEB, 0xD0, 50 });
    desenhar_topo(ctx);
    desenhar_baralho_e_vira(ctx);
    desenhar_assentos(ctx, vez);
    desenhar_cartas_na_mesa(ctx);

    if (p->estado == EST_VEZ_DE_JOGAR) {
        texto_centro(r, p->aviso, LARGURA / 2.0f, 488, TAM_TEXTO, COR_CREME);
        desenhar_bandeja(ctx, vez);
    }
    desenhar_botao_truco(ctx, vez);

    if (p->estado == EST_FIM_RODADA || p->estado == EST_FIM_MAO)
        desenhar_faixa(ctx);
    else if (p->estado == EST_RESPONDER_PEDIDO)
        desenhar_pedido(ctx);
    else if (p->estado == EST_DECIDIR_ONZE)
        desenhar_onze(ctx);

    if (ctx->mesa.aviso_disco && GetTime() - ctx->mesa.aviso_disco_inicio < 5.0) {
        Rectangle a = { 340, 76, 600, 36 };
        DrawRectangleRounded(a, 0.4f, 6, COR_PERIGO);
        texto_centro(r, p->falhou_var ? "Atenção: não consegui gravar o VAR."
                                      : "Atenção: não consegui salvar a caderneta.",
                     LARGURA / 2.0f, a.y + 7, TAM_TEXTO, COR_CREME);
    }
}
