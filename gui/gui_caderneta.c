/*
 * gui_caderneta.c — 3ª unidade, Pacote 7: caderneta (busca e lista A-Z)
 * Responsável: Lucca
 *
 * Caixa de busca no topo: enquanto digita, buscar_fregues procura o
 * nome (sem diferenciar maiúsculas) e, achando, abre a ficha num cartão
 * com os 5 números e a taxa de correu. Embaixo, a caderneta inteira em
 * ordem alfabética, com rolagem e as colunas da listagem do terminal.
 * A lista anda pela árvore em ordem, direto, sem montar vetor.
 */
#include <stdio.h>
#include <string.h>

#include "gui.h"

#define ALTURA_LINHA 36.0f

static Rectangle caixa_busca(void) { return (Rectangle){ 340, 92, 600, 52 }; }
static Rectangle area_lista(void)  { return (Rectangle){ 140, 352, 1000, 350 }; }

static const float COLUNA[] = { 160, 520, 630, 720, 820, 930, 1030 };
static const char *const TITULO_COLUNA[] = {
    "Nome", "Partidas", "Mãos", "Pediu", "Recebeu", "Correu", "Taxa"
};

/* k-ésima ficha em ordem alfabética (k começa em 0), ou NULL. */
static const Fregues *k_esimo(const NoCaderneta *no, int *k)
{
    const Fregues *f;

    if (no == NULL)
        return NULL;
    f = k_esimo(no->esq, k);
    if (f != NULL)
        return f;
    if ((*k)-- == 0)
        return &no->f;
    return k_esimo(no->dir, k);
}

static float rolagem_maxima(const Contexto *ctx)
{
    float total = ctx->caderneta.quantidade * ALTURA_LINHA;
    float h = area_lista().height - ALTURA_LINHA;
    return total > h ? total - h : 0.0f;
}

static void taxa_em_texto(const Fregues *f, char *s, size_t tam)
{
    double t = taxa_correu(f);

    if (t < 0)
        snprintf(s, tam, "--");
    else
        snprintf(s, tam, "%.1f%%", t * 100.0);
}

void tela_caderneta_abrir(Contexto *ctx)
{
    memset(&ctx->cad, 0, sizeof ctx->cad);
}

Tela tela_caderneta_atualizar(Contexto *ctx)
{
    EstadoCaderneta *e = &ctx->cad;
    Rectangle lista = area_lista();
    char limpo[MAX_NOME];
    Vector2 m = GetMousePosition();
    int k;

    if (botao_clicado(retangulo_voltar(), 1) || IsKeyPressed(KEY_ESCAPE))
        return TELA_MENU;

    editar_texto(e->busca, MAX_NOME);
    sem_espacos_nas_pontas(e->busca, limpo, MAX_NOME);
    if (limpo[0] != '\0') {
        e->ficha = buscar_fregues(&ctx->caderneta, limpo);
        e->nao_achou = (e->ficha == NULL);
    } else {
        e->nao_achou = 0;
    }

    if (mouse_em(lista))
        e->rolagem = rolar(e->rolagem, rolagem_maxima(ctx), ALTURA_LINHA * 2);

    /* clicar numa linha abre a ficha dela */
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && mouse_em(lista) &&
        m.y > lista.y + ALTURA_LINHA) {
        k = (int) ((m.y - lista.y - ALTURA_LINHA + e->rolagem) / ALTURA_LINHA);
        if (k >= 0 && k < ctx->caderneta.quantidade) {
            e->ficha = k_esimo(ctx->caderneta.raiz, &k);
            if (e->ficha != NULL) {
                snprintf(e->busca, sizeof e->busca, "%s", e->ficha->nome);
                e->nao_achou = 0;
            }
        }
    }
    return TELA_CADERNETA;
}

/* Desenha as linhas em ordem alfabética; *i conta a posição. */
static void desenhar_em_ordem(const Contexto *ctx, const NoCaderneta *no, int *i,
                              float y0)
{
    const Recursos *r = &ctx->rec;
    char num[24];
    float y;
    const Fregues *f;

    if (no == NULL)
        return;
    desenhar_em_ordem(ctx, no->esq, i, y0);

    y = y0 + *i * ALTURA_LINHA - ctx->cad.rolagem;
    f = &no->f;
    if (y > y0 - ALTURA_LINHA && y < area_lista().y + area_lista().height) {
        int marcada = (f == ctx->cad.ficha);
        if (marcada || (*i % 2) == 0)
            DrawRectangle(142, (int) y, 996, (int) ALTURA_LINHA,
                          marcada ? Fade(COR_PATO, 0.25f) : (Color){ 255, 255, 255, 12 });
        texto(r, f->nome, COLUNA[0], y + 7, TAM_TEXTO, COR_CREME);
        snprintf(num, sizeof num, "%d", f->partidas);
        texto(r, num, COLUNA[1], y + 7, TAM_TEXTO, COR_CREME);
        snprintf(num, sizeof num, "%d", f->maos_ganhas);
        texto(r, num, COLUNA[2], y + 7, TAM_TEXTO, COR_CREME);
        snprintf(num, sizeof num, "%d", f->trucos_pedidos);
        texto(r, num, COLUNA[3], y + 7, TAM_TEXTO, COR_CREME);
        snprintf(num, sizeof num, "%d", f->trucos_recebidos);
        texto(r, num, COLUNA[4], y + 7, TAM_TEXTO, COR_CREME);
        snprintf(num, sizeof num, "%d", f->correu);
        texto(r, num, COLUNA[5], y + 7, TAM_TEXTO, COR_CREME);
        taxa_em_texto(f, num, sizeof num);
        texto(r, num, COLUNA[6], y + 7, TAM_TEXTO, COR_CREME);
    }
    (*i)++;
    desenhar_em_ordem(ctx, no->dir, i, y0);
}

static void desenhar_ficha(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const Fregues *f = ctx->cad.ficha;
    Rectangle c = { 140, 160, 1000, 172 };
    static const char *const ROTULO[] = { "Partidas", "Mãos ganhas", "Trucos pedidos",
                                          "Recebidos", "Correu", "Taxa de correu" };
    char valor[6][24];
    int i;

    desenhar_painel(c);
    if (f == NULL) {
        texto_centro(r, ctx->cad.nao_achou ? "Esse aí nunca sentou aqui."
                                           : "Digite um nome ou clique numa linha da lista.",
                     LARGURA / 2.0f, c.y + 70, TAM_SUBTITULO,
                     ctx->cad.nao_achou ? COR_PERIGO : COR_CREME_FRACO);
        return;
    }
    texto(r, f->nome, c.x + 28, c.y + 18, 40, COR_PATO);
    snprintf(valor[0], 24, "%d", f->partidas);
    snprintf(valor[1], 24, "%d", f->maos_ganhas);
    snprintf(valor[2], 24, "%d", f->trucos_pedidos);
    snprintf(valor[3], 24, "%d", f->trucos_recebidos);
    snprintf(valor[4], 24, "%d", f->correu);
    taxa_em_texto(f, valor[5], 24);
    for (i = 0; i < 6; i++) {
        float x = c.x + 28 + i * 162.0f;
        texto(r, valor[i], x, c.y + 78, 44, i == 5 ? COR_PERIGO : COR_CREME);
        texto(r, ROTULO[i], x, c.y + 132, 18, COR_CREME_FRACO);
    }
}

void tela_caderneta_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    Rectangle lista = area_lista();
    char titulo[64];
    int i = 0, n = ctx->caderneta.quantidade;

    desenhar_feltro();
    snprintf(titulo, sizeof titulo, "Caderneta (%d fregu%s)", n, n == 1 ? "ês" : "eses");
    desenhar_cabecalho(r, titulo);
    desenhar_caixa_texto(r, caixa_busca(), ctx->cad.busca, 1, "Procurar freguês pelo nome");
    desenhar_ficha(ctx);

    desenhar_painel(lista);
    for (i = 0; i < 7; i++)
        texto(r, TITULO_COLUNA[i], COLUNA[i], lista.y + 8, TAM_TEXTO, COR_PATO);
    if (n == 0) {
        texto_centro(r, "A caderneta está vazia.", LARGURA / 2.0f, lista.y + 140,
                     TAM_SUBTITULO, COR_CREME_FRACO);
        return;
    }
    iniciar_recorte((Rectangle){ lista.x, lista.y + ALTURA_LINHA, lista.width,
                                 lista.height - ALTURA_LINHA - 4 });
    i = 0;
    desenhar_em_ordem(ctx, ctx->caderneta.raiz, &i, lista.y + ALTURA_LINHA);
    EndScissorMode();
}
