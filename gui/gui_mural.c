/*
 * gui_mural.c — 3ª unidade, Pacote 7: mural, mural da vergonha e fim
 * de partida
 * Responsável: Pedro
 *
 * Mural: um freguês por linha, do que menos ganhou mãos ao que mais
 * ganhou (como o PDF pede), ordenado pelo mergesort estável do mural.c,
 * com barra horizontal proporcional às mãos ganhas.
 * Mural da vergonha (aba ao lado): taxa de correu em barras vermelhas,
 * "--" para quem nunca recebeu pedido, e o cartão "Mão mais cara"
 * quando a tela foi aberta pelo fim de partida.
 * Fim de partida: "Dupla 1 venceu!", os dois nomes, placar final,
 * confete e os botões Ver VAR, Mural e Menu.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"

#define ALTURA_LINHA 42.0f

static Rectangle aba(int i)       { return (Rectangle){ 370.0f + i * 280.0f, 90, 260, 48 }; }
static Rectangle area_lista(const Contexto *ctx)
{
    int com_cartao = ctx->mural.aba == 1 && ctx->tela_anterior == TELA_FIM_PARTIDA &&
                     ctx->partida_ativa;
    float y = com_cartao ? 236.0f : 156.0f;
    return (Rectangle){ 140, y, 1000, 700 - y };
}

void tela_mural_liberar(Contexto *ctx)
{
    free(ctx->mural.vet);
    free(ctx->mural.alfabetica);
    ctx->mural.vet = ctx->mural.alfabetica = NULL;
    ctx->mural.n = 0;
}

void tela_mural_abrir(Contexto *ctx)
{
    EstadoMural *e = &ctx->mural;
    int n = ctx->caderneta.quantidade, i;

    tela_mural_liberar(ctx);
    /* vindo do fim de partida, abre direto na vergonha (mão mais cara) */
    e->aba = (ctx->tela_anterior == TELA_FIM_PARTIDA) ? 1 : 0;
    e->rolagem = 0;
    if (n == 0)
        return;

    e->vet = malloc((size_t) n * sizeof *e->vet);
    e->alfabetica = malloc((size_t) n * sizeof *e->alfabetica);
    if (e->vet == NULL || e->alfabetica == NULL) {
        tela_mural_liberar(ctx);
        return;
    }
    e->n = caderneta_para_vetor(&ctx->caderneta, e->alfabetica, n);
    for (i = 0; i < e->n; i++)
        e->vet[i] = e->alfabetica[i];
    ordenar_por_maos(e->vet, e->n); /* estável: empatados ficam A-Z */
}

Tela tela_mural_atualizar(Contexto *ctx)
{
    EstadoMural *e = &ctx->mural;
    Rectangle lista = area_lista(ctx);
    float total = e->n * ALTURA_LINHA, h = lista.height - ALTURA_LINHA;
    int i;

    if (botao_clicado(retangulo_voltar(), 1) || IsKeyPressed(KEY_ESCAPE)) {
        tela_mural_liberar(ctx);
        return ctx->tela_anterior == TELA_FIM_PARTIDA ? TELA_FIM_PARTIDA : TELA_MENU;
    }
    for (i = 0; i < 2; i++)
        if (botao_clicado(aba(i), 1)) {
            e->aba = i;
            e->rolagem = 0;
        }
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        e->aba = 1 - e->aba;
        e->rolagem = 0;
    }
    if (mouse_em(lista))
        e->rolagem = rolar(e->rolagem, total > h ? total - h : 0, ALTURA_LINHA * 2);
    return TELA_MURAL;
}

static void desenhar_mural(const Contexto *ctx, Rectangle lista)
{
    const Recursos *r = &ctx->rec;
    const EstadoMural *e = &ctx->mural;
    int i, maior = e->n > 0 ? e->vet[e->n - 1]->maos_ganhas : 0;
    char linha[160];

    texto(r, "#", 160, lista.y + 10, TAM_TEXTO, COR_PATO);
    texto(r, "Nome", 210, lista.y + 10, TAM_TEXTO, COR_PATO);
    texto(r, "Mãos ganhas", 470, lista.y + 10, TAM_TEXTO, COR_PATO);
    texto_direita(r, "Partidas", 1120, lista.y + 10, TAM_TEXTO, COR_PATO);

    iniciar_recorte((Rectangle){ lista.x, lista.y + ALTURA_LINHA, lista.width,
                                 lista.height - ALTURA_LINHA - 4 });
    for (i = 0; i < e->n; i++) {
        const Fregues *f = e->vet[i];
        float y = lista.y + ALTURA_LINHA + i * ALTURA_LINHA - e->rolagem;
        float w = maior > 0 ? 440.0f * f->maos_ganhas / maior : 0.0f;

        if (y < lista.y || y > lista.y + lista.height)
            continue;
        snprintf(linha, sizeof linha, "%d", i + 1);
        texto(r, linha, 160, y + 9, TAM_TEXTO, COR_CREME_FRACO);
        texto(r, f->nome, 210, y + 9, TAM_TEXTO, COR_CREME);
        DrawRectangleRounded((Rectangle){ 470, y + 8, 440, 24 }, 0.5f, 6,
                             (Color){ 255, 255, 255, 18 });
        if (w > 0)
            DrawRectangleRounded((Rectangle){ 470, y + 8, fmaxf(w, 24), 24 }, 0.5f, 6,
                                 COR_PATO);
        snprintf(linha, sizeof linha, "%d", f->maos_ganhas);
        texto(r, linha, 925, y + 9, TAM_TEXTO, COR_CREME);
        snprintf(linha, sizeof linha, "%d", f->partidas);
        texto_direita(r, linha, 1120, y + 9, TAM_TEXTO, COR_CREME);
    }
    EndScissorMode();
}

static void desenhar_vergonha(const Contexto *ctx, Rectangle lista)
{
    const Recursos *r = &ctx->rec;
    const EstadoMural *e = &ctx->mural;
    char linha[160];
    int i;

    texto(r, "Nome", 160, lista.y + 10, TAM_TEXTO, COR_PATO);
    texto(r, "Taxa de correu", 470, lista.y + 10, TAM_TEXTO, COR_PATO);
    texto_direita(r, "Correu/Recebeu", 1120, lista.y + 10, TAM_TEXTO, COR_PATO);

    iniciar_recorte((Rectangle){ lista.x, lista.y + ALTURA_LINHA, lista.width,
                                 lista.height - ALTURA_LINHA - 4 });
    for (i = 0; i < e->n; i++) {
        const Fregues *f = e->alfabetica[i];
        float y = lista.y + ALTURA_LINHA + i * ALTURA_LINHA - e->rolagem;
        double t = taxa_correu(f);

        if (y < lista.y || y > lista.y + lista.height)
            continue;
        texto(r, f->nome, 160, y + 9, TAM_TEXTO, COR_CREME);
        DrawRectangleRounded((Rectangle){ 470, y + 8, 400, 24 }, 0.5f, 6,
                             (Color){ 255, 255, 255, 18 });
        if (t < 0) {
            texto(r, "--", 885, y + 9, TAM_TEXTO, COR_CREME_FRACO);
        } else {
            if (t > 0)
                DrawRectangleRounded((Rectangle){ 470, y + 8, fmaxf(400.0f * (float) t, 24),
                                                  24 },
                                     0.5f, 6, COR_PERIGO);
            snprintf(linha, sizeof linha, "%.1f%%", t * 100.0);
            texto(r, linha, 885, y + 9, TAM_TEXTO, COR_CREME);
        }
        snprintf(linha, sizeof linha, "%d/%d", f->correu, f->trucos_recebidos);
        texto_direita(r, linha, 1120, y + 9, TAM_TEXTO, COR_CREME);
    }
    EndScissorMode();
}

/* Cartão "Mão mais cara" (só quando veio do fim de partida). */
static void desenhar_mao_mais_cara(const Contexto *ctx, Rectangle onde)
{
    const Recursos *r = &ctx->rec;
    const MaoMaisCara *m = &ctx->partida.mais_cara;
    char linha[256];

    desenhar_painel(onde);
    texto(r, "Mão mais cara da partida", onde.x + 24, onde.y + 12, TAM_TEXTO, COR_PATO);
    if (!m->existe)
        snprintf(linha, sizeof linha, "Nenhuma mão valeu ponto.");
    else
        snprintf(linha, sizeof linha, "Mão %d · %d ponto%s · Dupla %d (%s e %s)",
                 m->num_mao, m->pontos, m->pontos == 1 ? "" : "s", m->dupla,
                 m->nomes[0], m->nomes[1]);
    texto(r, linha, onde.x + 24, onde.y + 40, TAM_SUBTITULO, COR_CREME);
}

void tela_mural_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const EstadoMural *e = &ctx->mural;
    Rectangle lista = area_lista(ctx);

    desenhar_feltro();
    desenhar_cabecalho(r, e->aba == 0 ? "Mural do Bar do Pato" : "Mural da vergonha");
    desenhar_botao(r, aba(0), "Mural", e->aba == 0 ? BOTAO_PRINCIPAL : BOTAO_NORMAL, 1);
    desenhar_botao(r, aba(1), "Mural da vergonha",
                   e->aba == 1 ? BOTAO_PERIGO : BOTAO_NORMAL, 1);
    if (e->aba == 1 && lista.y > 200)
        desenhar_mao_mais_cara(ctx, (Rectangle){ 140, 150, 1000, 78 });

    desenhar_painel(lista);
    if (e->n == 0) {
        texto_centro(r, ctx->caderneta.quantidade > 0 ? "Faltou memória para montar o mural."
                                                     : "Ninguém sentou na mesa ainda.",
                     LARGURA / 2.0f, lista.y + 120, TAM_SUBTITULO, COR_CREME_FRACO);
        return;
    }
    if (e->aba == 0)
        desenhar_mural(ctx, lista);
    else
        desenhar_vergonha(ctx, lista);
}

/* ================================================================== */
/* Fim de partida                                                      */
/* ================================================================== */

static Rectangle bot_fim(int i) { return (Rectangle){ 330.0f + i * 220.0f, 600, 200, 64 }; }

static void soltar_confete(Confete *c, int no_alto)
{
    static const Color cores[] = { COR_PATO, COR_PERIGO, COR_CREME,
                                   { 0x4C, 0xB5, 0x77, 255 }, { 0x3A, 0x8E, 0xD8, 255 } };

    c->pos = (Vector2){ (float) GetRandomValue(0, LARGURA),
                        no_alto ? (float) GetRandomValue(-ALTURA, -10)
                                : (float) GetRandomValue(-40, ALTURA) };
    c->vel = (Vector2){ (float) GetRandomValue(-60, 60), (float) GetRandomValue(80, 220) };
    c->giro = (float) GetRandomValue(0, 360);
    c->vel_giro = (float) GetRandomValue(-360, 360);
    c->cor = cores[GetRandomValue(0, 4)];
}

void tela_fim_abrir(Contexto *ctx)
{
    int i;

    ctx->fim.inicio = GetTime();
    for (i = 0; i < MAX_CONFETES; i++)
        soltar_confete(&ctx->fim.confetes[i], 1);
}

Tela tela_fim_atualizar(Contexto *ctx)
{
    float dt = GetFrameTime();
    int i, ainda_chove = GetTime() - ctx->fim.inicio < 6.0;

    if (!ctx->partida_ativa)
        return TELA_MENU;
    for (i = 0; i < MAX_CONFETES; i++) {
        Confete *c = &ctx->fim.confetes[i];
        c->pos.x += c->vel.x * dt + sinf((float) GetTime() * 2 + i) * 0.6f;
        c->pos.y += c->vel.y * dt;
        c->giro += c->vel_giro * dt;
        if (c->pos.y > ALTURA + 20 && ainda_chove)
            soltar_confete(c, 1);
    }

    if (botao_clicado(bot_fim(0), 1) || IsKeyPressed(KEY_V)) {
        ctx->var_aberto = ctx->partida.historico;
        ctx->var.var_proprio = 0;
        ctx->var.id_antigo = 0;
        return TELA_VAR;
    }
    if (botao_clicado(bot_fim(1), 1) || IsKeyPressed(KEY_M))
        return TELA_MURAL;
    if (botao_clicado(bot_fim(2), 1) || IsKeyPressed(KEY_ESCAPE) ||
        IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        fechar_partida_ativa(ctx);
        return TELA_MENU;
    }
    return TELA_FIM_PARTIDA;
}

void tela_fim_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Partida *p = &ctx->partida;
    const Dupla *d;
    char linha[256];
    float t = progresso(ctx->fim.inicio, 0.6f);
    int i;

    if (!ctx->partida_ativa)
        return;
    d = &p->duplas[p->campea];

    desenhar_feltro();
    for (i = 0; i < MAX_CONFETES; i++) {
        const Confete *c = &ctx->fim.confetes[i];
        DrawRectanglePro((Rectangle){ c->pos.x, c->pos.y, 10, 6 }, (Vector2){ 5, 3 },
                         c->giro, c->cor);
    }

    DrawTexturePro(r->logo, (Rectangle){ 0, 0, (float) r->logo.width, (float) r->logo.height },
                   (Rectangle){ 570, 40, 140, 140 }, (Vector2){ 0, 0 }, 0, WHITE);
    snprintf(linha, sizeof linha, "Dupla %d venceu!", p->campea + 1);
    texto_centro(r, linha, LARGURA / 2.0f, 200, 40.0f + 32.0f * t, COR_PATO);
    snprintf(linha, sizeof linha, "%s e %s", d->jogadores[0]->nome, d->jogadores[1]->nome);
    texto_centro(r, linha, LARGURA / 2.0f, 290, 40, COR_CREME);

    snprintf(linha, sizeof linha, "%d  ×  %d", p->duplas[0].pontos, p->duplas[1].pontos);
    texto_centro(r, linha, LARGURA / 2.0f, 360, 64, COR_CREME);
    snprintf(linha, sizeof linha, "Dupla 1: %s & %s          Dupla 2: %s & %s",
             p->duplas[0].jogadores[0]->nome, p->duplas[0].jogadores[1]->nome,
             p->duplas[1].jogadores[0]->nome, p->duplas[1].jogadores[1]->nome);
    texto_centro(r, linha, LARGURA / 2.0f, 440, TAM_TEXTO, COR_CREME_FRACO);

    if (p->mais_cara.existe)
        snprintf(linha, sizeof linha, "Mão mais cara: mão %d, %d ponto%s (Dupla %d)",
                 p->mais_cara.num_mao, p->mais_cara.pontos,
                 p->mais_cara.pontos == 1 ? "" : "s", p->mais_cara.dupla);
    else
        snprintf(linha, sizeof linha, "Nenhuma mão valeu ponto.");
    texto_centro(r, linha, LARGURA / 2.0f, 490, TAM_SUBTITULO, COR_PATO);
    snprintf(linha, sizeof linha, "%d rodadas no VAR · partida nº %d",
             tamanho_historico(p->historico), p->id_partida);
    texto_centro(r, linha, LARGURA / 2.0f, 534, TAM_TEXTO, COR_CREME_FRACO);

    desenhar_botao(r, bot_fim(0), "Ver VAR (V)", BOTAO_NORMAL, 1);
    desenhar_botao(r, bot_fim(1), "Mural (M)", BOTAO_NORMAL, 1);
    desenhar_botao(r, bot_fim(2), "Menu", BOTAO_PRINCIPAL, 1);
}
