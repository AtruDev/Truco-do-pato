/*
 * gui_menu.c — 3ª unidade, Pacote 7: menu inicial e cadastro
 * Responsável: Lucca
 *
 * Menu: logo do Bar do Pato, quantos fregueses tem a caderneta e os
 * botões grandes (Continuar partida, se houver uma ativa).
 * Cadastro: duas colunas (Dupla 1, Dupla 2) com 2 caixas cada. Enquanto
 * digita, buscar_fregues mostra "Rosto novo" ou "Freguês da casa: ...",
 * e problema_no_nome aparece em vermelho embaixo da caixa. "Sentar à
 * mesa" só acende com os 4 nomes válidos.
 */
#include <stdio.h>
#include <string.h>

#include "gui.h"

/* ================================================================== */
/* Menu                                                                */
/* ================================================================== */

typedef enum {
    OP_CONTINUAR, OP_NOVA, OP_CADERNETA, OP_MURAL, OP_ANTIGAS, OP_SAIR, NUM_OPCOES
} OpcaoMenu;

static const char *const ROTULO_MENU[NUM_OPCOES] = {
    "Continuar partida", "Nova partida", "Caderneta", "Mural do bar",
    "VAR de partidas antigas", "Fechar o bar"
};

/* Retângulo do botão da opção; o Continuar só existe com partida ativa. */
static Rectangle botao_menu(const Contexto *ctx, int op)
{
    int linha = ctx->partida_ativa ? op : op - 1;
    return (Rectangle){ 720, 170.0f + linha * 78.0f, 420, 62 };
}

static Tela tela_da_opcao(Contexto *ctx, int op)
{
    switch (op) {
    case OP_CONTINUAR: return TELA_MESA;
    case OP_NOVA:      return TELA_CADASTRO;
    case OP_CADERNETA: return TELA_CADERNETA;
    case OP_MURAL:     return TELA_MURAL;
    case OP_ANTIGAS:   return TELA_PARTIDAS_ANTIGAS;
    default:           (void) ctx; return TELA_SAIR;
    }
}

void tela_menu_abrir(Contexto *ctx)
{
    (void) ctx;
}

Tela tela_menu_atualizar(Contexto *ctx)
{
    int op, primeira = ctx->partida_ativa ? OP_CONTINUAR : OP_NOVA;

    for (op = primeira; op < NUM_OPCOES; op++)
        if (botao_clicado(botao_menu(ctx, op), 1))
            return tela_da_opcao(ctx, op);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
        return tela_da_opcao(ctx, primeira);
    return TELA_MENU;
}

void tela_menu_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    char linha[160];
    int op, n = ctx->caderneta.quantidade;
    EstiloBotao estilo;

    desenhar_feltro();
    DrawTexturePro(r->logo, (Rectangle){ 0, 0, (float) r->logo.width,
                                          (float) r->logo.height },
                   (Rectangle){ 170, 110, 300, 300 }, (Vector2){ 0, 0 }, 0, WHITE);
    texto_centro(r, "Truco no", 320, 430, 40, COR_CREME);
    texto_centro(r, "Bar do Pato", 320, 474, 64, COR_PATO);
    snprintf(linha, sizeof linha, "%d fregu%s na caderneta", n,
             n == 1 ? "ês" : "eses");
    texto_centro(r, linha, 320, 556, TAM_SUBTITULO, COR_CREME_FRACO);

    for (op = ctx->partida_ativa ? OP_CONTINUAR : OP_NOVA; op < NUM_OPCOES; op++) {
        if (op == OP_SAIR)
            estilo = BOTAO_PERIGO;
        else if (op == (ctx->partida_ativa ? OP_CONTINUAR : OP_NOVA))
            estilo = BOTAO_PRINCIPAL;
        else
            estilo = BOTAO_NORMAL;
        desenhar_botao(r, botao_menu(ctx, op), ROTULO_MENU[op], estilo, 1);
    }
    texto(r, "F11: tela cheia", 20, ALTURA - 34, 18, COR_CREME_FRACO);
}

/* ================================================================== */
/* Cadastro                                                            */
/* ================================================================== */

static Rectangle caixa_nome(int i)
{
    float x = (i < 2) ? 150.0f : 690.0f;
    return (Rectangle){ x, 230.0f + (i % 2) * 170.0f, 440, 54 };
}

static Rectangle botao_sentar(void)
{
    return (Rectangle){ LARGURA / 2.0f - 170, 600, 340, 70 };
}

/* Nomes sem espaços nas pontas e o problema de cada um (NULL = serve).
 * Cada nome é comparado com os que vêm antes dele. Retorna 1 se os 4
 * servem. */
static int conferir_nomes(const EstadoCadastro *e, char limpos[][MAX_NOME],
                          const char *problema[])
{
    const char *aceitos[NUM_JOGADORES];
    int i, todos = 1;

    for (i = 0; i < NUM_JOGADORES; i++) {
        sem_espacos_nas_pontas(e->nomes[i], limpos[i], MAX_NOME);
        aceitos[i] = limpos[i];
    }
    for (i = 0; i < NUM_JOGADORES; i++) {
        problema[i] = problema_no_nome(limpos[i], aceitos, i);
        if (problema[i] != NULL)
            todos = 0;
    }
    return todos;
}

void tela_cadastro_abrir(Contexto *ctx)
{
    memset(&ctx->cadastro, 0, sizeof ctx->cadastro);
}

static Tela sentar(Contexto *ctx, char limpos[][MAX_NOME])
{
    const char *nomes[NUM_JOGADORES];
    int i;

    for (i = 0; i < NUM_JOGADORES; i++)
        nomes[i] = limpos[i];
    fechar_partida_ativa(ctx); /* a partida abandonada já está contada */
    if (!iniciar_partida(&ctx->partida, nomes, &ctx->caderneta)) {
        ctx->cadastro.erro_memoria = 1;
        return TELA_CADASTRO;
    }
    ctx->partida_ativa = 1;
    ctx->cadastro.foco = -1; /* sinal para a mesa: partida nova, zera a tela */
    return TELA_MESA;
}

Tela tela_cadastro_atualizar(Contexto *ctx)
{
    EstadoCadastro *e = &ctx->cadastro;
    char limpos[NUM_JOGADORES][MAX_NOME];
    const char *problema[NUM_JOGADORES];
    int i, prontos;

    if (botao_clicado(retangulo_voltar(), 1) || IsKeyPressed(KEY_ESCAPE))
        return TELA_MENU;

    for (i = 0; i < NUM_JOGADORES; i++)
        if (mouse_em(caixa_nome(i)) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            e->foco = i;

    editar_texto(e->nomes[e->foco], MAX_NOME);
    prontos = conferir_nomes(e, limpos, problema);

    if (IsKeyPressed(KEY_TAB)) {
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            e->foco = (e->foco + NUM_JOGADORES - 1) % NUM_JOGADORES;
        else
            e->foco = (e->foco + 1) % NUM_JOGADORES;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        if (prontos)
            return sentar(ctx, limpos);
        e->foco = (e->foco + 1) % NUM_JOGADORES;
    }
    if (botao_clicado(botao_sentar(), prontos))
        return sentar(ctx, limpos);
    return TELA_CADASTRO;
}

void tela_cadastro_desenhar(const Contexto *ctx)
{
    const Recursos *r = &ctx->rec;
    const EstadoCadastro *e = &ctx->cadastro;
    char limpos[NUM_JOGADORES][MAX_NOME];
    const char *problema[NUM_JOGADORES];
    char rotulo[96];
    const Fregues *f;
    Rectangle cx;
    int i, prontos = conferir_nomes(e, limpos, problema);

    desenhar_feltro();
    desenhar_cabecalho(r, "Quem senta à mesa?");

    for (i = 0; i < 2; i++) {
        Rectangle painel = { i == 0 ? 120.0f : 660.0f, 110, 500, 450 };
        desenhar_painel(painel);
        snprintf(rotulo, sizeof rotulo, "Dupla %d", i + 1);
        texto_centro(r, rotulo, painel.x + painel.width / 2, painel.y + 18,
                     TAM_SUBTITULO + 4, COR_PATO);
    }

    for (i = 0; i < NUM_JOGADORES; i++) {
        cx = caixa_nome(i);
        snprintf(rotulo, sizeof rotulo, "Jogador %d  ·  cadeira %d", i % 2 + 1,
                 (i % 2) * 2 + (i / 2) + 1);
        texto(r, rotulo, cx.x, cx.y - 30, TAM_TEXTO, COR_CREME_FRACO);
        desenhar_caixa_texto(r, cx, e->nomes[i], e->foco == i, "Digite o nome");

        if (e->nomes[i][0] == '\0')
            continue;
        if (problema[i] != NULL) {
            texto(r, problema[i], cx.x, cx.y + cx.height + 10, TAM_TEXTO, COR_PERIGO);
            continue;
        }
        f = buscar_fregues(&ctx->caderneta, limpos[i]);
        if (f == NULL)
            snprintf(rotulo, sizeof rotulo, "Rosto novo");
        else
            snprintf(rotulo, sizeof rotulo, "Freguês da casa: %d partida%s, %d mão%s",
                     f->partidas, f->partidas == 1 ? "" : "s", f->maos_ganhas,
                     f->maos_ganhas == 1 ? "" : "s");
        texto(r, rotulo, cx.x, cx.y + cx.height + 10, TAM_TEXTO,
              f == NULL ? COR_CREME : COR_PATO);
    }

    texto_centro(r, "Parceiros sentam frente a frente. Tab troca de caixa, Enter senta.",
                 LARGURA / 2.0f, 572, 18, COR_CREME_FRACO);
    if (e->erro_memoria)
        texto_centro(r, "Faltou memória para começar a partida.", LARGURA / 2.0f,
                     540, TAM_TEXTO, COR_PERIGO);
    desenhar_botao(r, botao_sentar(), "Sentar à mesa", BOTAO_PRINCIPAL, prontos);
}
