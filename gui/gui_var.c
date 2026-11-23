/*
 * gui_var.c — 3ª unidade, Pacote 7: VAR e VAR de partidas antigas
 * Responsável: Rafael
 *
 * VAR: uma rodada por vez, com as cartas na ordem em que saíram e o
 * nome embaixo de cada uma, as apostas em fichinhas ("Ana pediu
 * truco", "Caio correu") e o resultado. Embaixo, a linha do tempo: um
 * tracinho por rodada, agrupados por mão, o atual em amarelo. Clicar
 * num tracinho pula para ele; ◀ ▶ e as setas do teclado andam uma
 * rodada. Anda pela lista duplamente encadeada com primeira_rodada /
 * ultima_rodada e ->prox / ->ant (navegar_var imprime no terminal).
 *
 * Partidas antigas: lista vinda de ler_partidas_salvas (data, duplas,
 * placar ou "interrompida"), a mais nova em cima. Clicar abre o VAR
 * dela via carregar_partida; essa lista é desta tela e é liberada ao
 * sair do VAR.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"

#define X_LINHA_TEMPO  90.0f
#define L_LINHA_TEMPO  1100.0f
#define Y_LINHA_TEMPO  628.0f
#define VAO_ENTRE_MAOS 10.0f

static Rectangle bot_anterior(void) { return (Rectangle){ 40, 270, 76, 96 }; }
static Rectangle bot_proximo(void)  { return (Rectangle){ 1164, 270, 76, 96 }; }

/* ------------------------------------------------------------------ */
/* Linha do tempo                                                      */
/* ------------------------------------------------------------------ */

/* Largura de cada tracinho para caber tudo na linha. */
static float largura_tick(const Historico *h)
{
    const NoHist *no;
    int n = 0, maos = 0, ultima = -1;
    float w;

    for (no = primeira_rodada(h); no != NULL; no = no->prox) {
        n++;
        if (no->num_mao != ultima) {
            maos++;
            ultima = no->num_mao;
        }
    }
    if (n == 0)
        return 0;
    w = (L_LINHA_TEMPO - (maos - 1) * VAO_ENTRE_MAOS) / n;
    return w > 36.0f ? 36.0f : w;
}

/* Retângulo do tracinho do nó 'alvo' (percorre a lista até ele). */
static Rectangle tick_de(const Historico *h, const NoHist *alvo, float w)
{
    const NoHist *no;
    float x = X_LINHA_TEMPO;
    int ultima = -1;

    for (no = primeira_rodada(h); no != NULL; no = no->prox) {
        if (ultima != -1 && no->num_mao != ultima)
            x += VAO_ENTRE_MAOS;
        ultima = no->num_mao;
        if (no == alvo)
            break;
        x += w;
    }
    return (Rectangle){ x + 1, Y_LINHA_TEMPO, w - 2 > 2 ? w - 2 : 2, 30 };
}

/* ------------------------------------------------------------------ */
/* VAR                                                                 */
/* ------------------------------------------------------------------ */

void tela_var_liberar(Contexto *ctx)
{
    if (ctx->var.var_proprio && ctx->var_aberto != NULL)
        liberar_historico(ctx->var_aberto);
    if (ctx->var.var_proprio)
        ctx->var_aberto = NULL;
    ctx->var.var_proprio = 0;
}

void tela_var_abrir(Contexto *ctx)
{
    /* começa pela rodada mais recente, como no terminal */
    ctx->var.atual = ultima_rodada(ctx->var_aberto);
    ctx->var.pos = tamanho_historico(ctx->var_aberto);
}

Tela tela_var_atualizar(Contexto *ctx)
{
    EstadoVar *e = &ctx->var;
    const NoHist *no;
    float w;
    int k;

    if (botao_clicado(retangulo_voltar(), 1) || IsKeyPressed(KEY_ESCAPE) ||
        IsKeyPressed(KEY_V)) {
        Tela volta = ctx->tela_anterior;
        tela_var_liberar(ctx);
        if (volta != TELA_MESA && volta != TELA_FIM_PARTIDA &&
            volta != TELA_PARTIDAS_ANTIGAS)
            volta = TELA_MENU;
        return volta;
    }
    if (e->atual == NULL)
        return TELA_VAR;

    if ((botao_clicado(bot_anterior(), e->atual->ant != NULL) ||
         IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && e->atual->ant != NULL) {
        e->atual = e->atual->ant;
        e->pos--;
    }
    if ((botao_clicado(bot_proximo(), e->atual->prox != NULL) ||
         IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) &&
        e->atual->prox != NULL) {
        e->atual = e->atual->prox;
        e->pos++;
    }
    if (IsKeyPressed(KEY_HOME)) {
        e->atual = primeira_rodada(ctx->var_aberto);
        e->pos = 1;
    }
    if (IsKeyPressed(KEY_END)) {
        e->atual = ultima_rodada(ctx->var_aberto);
        e->pos = tamanho_historico(ctx->var_aberto);
    }

    /* clicar num tracinho pula para ele */
    w = largura_tick(ctx->var_aberto);
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        for (no = primeira_rodada(ctx->var_aberto), k = 1; no != NULL; no = no->prox, k++) {
            Rectangle t = tick_de(ctx->var_aberto, no, w);
            t.y -= 8;
            t.height += 16;
            if (mouse_em(t)) {
                e->atual = no;
                e->pos = k;
                break;
            }
        }
    return TELA_VAR;
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

static void desenhar_rodada(const Contexto *ctx, const NoHist *no)
{
    const Recursos *r = &ctx->rec;
    char linha[256];
    float x0, x, w, total;
    int i, correu = 0;

    snprintf(linha, sizeof linha, "Mão %d  ·  Rodada %d", no->num_mao, no->num_rodada);
    texto_centro(r, linha, LARGURA / 2.0f, 100, 40, COR_PATO);
    snprintf(linha, sizeof linha, "%d de %d", ctx->var.pos,
             tamanho_historico(ctx->var_aberto));
    texto_centro(r, linha, LARGURA / 2.0f, 150, TAM_TEXTO, COR_CREME_FRACO);

    /* cartas na ordem em que saíram, com o nome embaixo */
    if (no->rodada.qtd == 0) {
        texto_centro(r, "(nenhuma carta jogada)", LARGURA / 2.0f, 300, TAM_SUBTITULO,
                     COR_CREME_FRACO);
    }
    x0 = 640.0f - (no->rodada.qtd - 1) * 190.0f / 2.0f;
    for (i = 0; i < no->rodada.qtd; i++) {
        Rectangle c = retangulo_carta(x0 + i * 190.0f, 318, 190);
        desenhar_carta(r, no->rodada.cartas[i], c, 0, 0);
        snprintf(linha, sizeof linha, "%d. %s", i + 1, no->nomes[i]);
        texto_centro(r, linha, c.x + c.width / 2, c.y + c.height + 10, TAM_TEXTO,
                     COR_CREME);
    }

    /* apostas em fichinhas */
    total = 0;
    for (i = 0; i < no->qtd_eventos; i++) {
        snprintf(linha, sizeof linha, "%s %s", no->eventos[i].nome,
                 texto_evento(no->eventos[i].tipo));
        total += largura_texto(r, linha, 18) + 40;
    }
    x = 640.0f - total / 2.0f;
    for (i = 0; i < no->qtd_eventos; i++) {
        const EventoAposta *ev = &no->eventos[i];
        Color cor = ev->tipo == ACEITOU ? COR_PATO
                    : ev->tipo == CORREU ? (Color){ 120, 110, 100, 255 } : COR_PERIGO;
        snprintf(linha, sizeof linha, "%s %s", ev->nome, texto_evento(ev->tipo));
        w = largura_texto(r, linha, 18) + 28;
        DrawRectangleRounded((Rectangle){ x, 470, w, 32 }, 0.6f, 8, cor);
        texto(r, linha, x + 14, 476, 18, ev->tipo == ACEITOU ? COR_PAINEL : COR_CREME);
        if (ev->tipo == CORREU)
            correu = 1;
        x += w + 12;
    }

    if (correu)
        snprintf(linha, sizeof linha, "Rodada interrompida: alguém correu (Dupla %d leva)",
                 no->rodada.dupla_vencedora + 1);
    else if (no->rodada.dupla_vencedora == RODADA_EMPATADA)
        snprintf(linha, sizeof linha, "Resultado: empate");
    else
        snprintf(linha, sizeof linha, "Resultado: Dupla %d levou",
                 no->rodada.dupla_vencedora + 1);
    texto_centro(r, linha, LARGURA / 2.0f, 530, TAM_SUBTITULO, COR_CREME);
}

static void desenhar_linha_do_tempo(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Historico *h = ctx->var_aberto;
    const NoHist *no;
    float w = largura_tick(h);
    char num[16];
    int ultima = -1;

    texto(r, "Linha do tempo", X_LINHA_TEMPO, Y_LINHA_TEMPO - 30, 18, COR_CREME_FRACO);
    for (no = primeira_rodada(h); no != NULL; no = no->prox) {
        Rectangle t = tick_de(h, no, w);
        Color cor = no == ctx->var.atual ? COR_PATO
                    : mouse_em(t) ? COR_CREME : (Color){ 0xF4, 0xEB, 0xD0, 110 };
        DrawRectangleRounded(t, 0.4f, 4, cor);
        if (no->num_mao != ultima) {
            ultima = no->num_mao;
            snprintf(num, sizeof num, "%d", no->num_mao);
            if (w >= 10 || no->num_mao % 5 == 1)
                texto(r, num, t.x, t.y + 34, 16, COR_CREME_FRACO);
        }
    }
}

void tela_var_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const EstadoVar *e = &ctx->var;
    char titulo[64];

    desenhar_feltro();
    if (e->id_antigo > 0)
        snprintf(titulo, sizeof titulo, "VAR · partida nº %d", e->id_antigo);
    else
        snprintf(titulo, sizeof titulo, "VAR da partida");
    desenhar_cabecalho(r, titulo);

    if (e->atual == NULL) {
        texto_centro(r, "O VAR ainda está vazio.", LARGURA / 2.0f, 330, TAM_TITULO,
                     COR_CREME_FRACO);
        return;
    }
    desenhar_rodada(ctx, e->atual);
    desenhar_botao(r, bot_anterior(), "\xE2\x97\x80", BOTAO_NORMAL, e->atual->ant != NULL);
    desenhar_botao(r, bot_proximo(), "\xE2\x96\xB6", BOTAO_NORMAL, e->atual->prox != NULL);
    desenhar_linha_do_tempo(ctx);
    texto_direita(r, "← → anda · Home/End · V ou Esc volta", 1190, 690, 16,
                  COR_CREME_FRACO);
}

/* ================================================================== */
/* Partidas antigas                                                    */
/* ================================================================== */

#define ALTURA_LINHA 58.0f

static Rectangle area_partidas(void) { return (Rectangle){ 120, 100, 1040, 600 }; }

void tela_partidas_liberar(Contexto *ctx)
{
    free(ctx->antigas.vet);
    ctx->antigas.vet = NULL;
    ctx->antigas.n = 0;
}

void tela_partidas_abrir(Contexto *ctx)
{
    EstadoPartidas *e = &ctx->antigas;
    int max = proximo_id_partida(ARQUIVO_HISTORICO) - 1;

    tela_partidas_liberar(ctx);
    e->rolagem = 0;
    e->nao_abriu = 0;
    if (max <= 0)
        return;
    /* os números são únicos e vão até max: cabem todas */
    e->vet = malloc((size_t) max * sizeof *e->vet);
    if (e->vet == NULL)
        return;
    e->n = ler_partidas_salvas(ARQUIVO_HISTORICO, e->vet, max);
}

/* Linha i da tela mostra a partida vet[n-1-i] (a mais nova em cima). */
static Rectangle linha_partida(const EstadoPartidas *e, int i)
{
    Rectangle a = area_partidas();
    return (Rectangle){ a.x + 10, a.y + 10 + i * ALTURA_LINHA - e->rolagem,
                        a.width - 20, ALTURA_LINHA - 8 };
}

Tela tela_partidas_atualizar(Contexto *ctx)
{
    EstadoPartidas *e = &ctx->antigas;
    Rectangle a = area_partidas();
    float total = e->n * ALTURA_LINHA + 20;
    int i;

    if (botao_clicado(retangulo_voltar(), 1) || IsKeyPressed(KEY_ESCAPE)) {
        tela_partidas_liberar(ctx);
        return TELA_MENU;
    }
    if (mouse_em(a))
        e->rolagem = rolar(e->rolagem, total > a.height ? total - a.height : 0,
                           ALTURA_LINHA * 2);

    for (i = 0; i < e->n; i++) {
        Rectangle l = linha_partida(e, i);
        if (l.y + l.height < a.y || l.y > a.y + a.height)
            continue;
        if (botao_clicado(l, 1)) {
            const ResumoPartida *rp = &e->vet[e->n - 1 - i];
            Historico *h = carregar_partida(ARQUIVO_HISTORICO, rp->id);

            if (h == NULL) {
                e->nao_abriu = rp->id;
                return TELA_PARTIDAS_ANTIGAS;
            }
            tela_var_liberar(ctx);
            ctx->var_aberto = h;
            ctx->var.var_proprio = 1;
            ctx->var.id_antigo = rp->id;
            return TELA_VAR;
        }
    }
    return TELA_PARTIDAS_ANTIGAS;
}

void tela_partidas_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const EstadoPartidas *e = &ctx->antigas;
    Rectangle a = area_partidas();
    char linha[256];
    int i;

    desenhar_feltro();
    desenhar_cabecalho(r, "VAR de partidas antigas");
    desenhar_painel(a);

    if (e->n == 0) {
        texto_centro(r, "Nenhuma partida salva.", LARGURA / 2.0f, 360, TAM_TITULO,
                     COR_CREME_FRACO);
        return;
    }

    iniciar_recorte((Rectangle){ a.x, a.y + 4, a.width, a.height - 8 });
    for (i = 0; i < e->n; i++) {
        const ResumoPartida *rp = &e->vet[e->n - 1 - i];
        Rectangle l = linha_partida(e, i);
        int sobre = mouse_em(l) && mouse_em(a);

        if (l.y + l.height < a.y || l.y > a.y + a.height)
            continue;
        DrawRectangleRounded(l, 0.25f, 6, sobre ? Fade(COR_PATO, 0.25f)
                                               : (Color){ 255, 255, 255, 14 });
        snprintf(linha, sizeof linha, "nº %d", rp->id);
        texto(r, linha, l.x + 16, l.y + 13, TAM_TEXTO + 2, COR_PATO);
        texto(r, rp->data, l.x + 110, l.y + 15, TAM_TEXTO, COR_CREME_FRACO);
        snprintf(linha, sizeof linha, "%s e %s  ×  %s e %s", rp->nomes[0], rp->nomes[2],
                 rp->nomes[1], rp->nomes[3]);
        texto(r, linha, l.x + 300, l.y + 13, TAM_TEXTO + 2, COR_CREME);
        if (rp->terminou) {
            snprintf(linha, sizeof linha, "%d × %d", rp->pontos_d1, rp->pontos_d2);
            texto_direita(r, linha, l.x + l.width - 20, l.y + 11, TAM_SUBTITULO,
                          COR_CREME);
        } else {
            texto_direita(r, "interrompida", l.x + l.width - 20, l.y + 15, TAM_TEXTO,
                          COR_PERIGO);
        }
    }
    EndScissorMode();

    if (e->nao_abriu > 0) {
        snprintf(linha, sizeof linha, "Não achei a partida %d.", e->nao_abriu);
        texto_centro(r, linha, LARGURA / 2.0f, 690, TAM_TEXTO, COR_PERIGO);
    }
}
