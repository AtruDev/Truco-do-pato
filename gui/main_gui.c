/*
 * main_gui.c — 3ª unidade, Pacote 7 (versão 2): janela e laço principal
 * Responsável: Arthur
 *
 * Só faz: abrir a janela, ir para a pasta do .exe (para achar assets/
 * mesmo abrindo com dois cliques), carregar recursos e caderneta, e
 * repetir atualizar + desenhar da tela atual a 60 FPS. Quando a tela
 * muda, chama o abrir da nova. Fechar a janela (ou "Fechar o bar")
 * salva a caderneta antes de sair.
 *
 * F11 alterna tela cheia. A área lógica é sempre 1280x720.
 */
#include <stdlib.h>
#include <time.h>

#include "gui.h"

void fechar_partida_ativa(Contexto *ctx)
{
    if (ctx->partida_ativa) {
        if (ctx->var_aberto == ctx->partida.historico)
            ctx->var_aberto = NULL;
        encerrar_partida(&ctx->partida);
        ctx->partida_ativa = 0;
    }
}

static void abrir_tela(Contexto *ctx, Tela t)
{
    switch (t) {
    case TELA_MENU:             tela_menu_abrir(ctx); break;
    case TELA_CADASTRO:         tela_cadastro_abrir(ctx); break;
    case TELA_MESA:             tela_mesa_abrir(ctx); break;
    case TELA_FIM_PARTIDA:      tela_fim_abrir(ctx); break;
    case TELA_CADERNETA:        tela_caderneta_abrir(ctx); break;
    case TELA_MURAL:            tela_mural_abrir(ctx); break;
    case TELA_VAR:              tela_var_abrir(ctx); break;
    case TELA_PARTIDAS_ANTIGAS: tela_partidas_abrir(ctx); break;
    default: break;
    }
}

static Tela atualizar_tela(Contexto *ctx)
{
    switch (ctx->tela) {
    case TELA_MENU:             return tela_menu_atualizar(ctx);
    case TELA_CADASTRO:         return tela_cadastro_atualizar(ctx);
    case TELA_MESA:             return tela_mesa_atualizar(ctx);
    case TELA_FIM_PARTIDA:      return tela_fim_atualizar(ctx);
    case TELA_CADERNETA:        return tela_caderneta_atualizar(ctx);
    case TELA_MURAL:            return tela_mural_atualizar(ctx);
    case TELA_VAR:              return tela_var_atualizar(ctx);
    case TELA_PARTIDAS_ANTIGAS: return tela_partidas_atualizar(ctx);
    default:                    return TELA_SAIR;
    }
}

static void desenhar_tela(const Contexto *ctx)
{
    switch (ctx->tela) {
    case TELA_MENU:             tela_menu_desenhar(ctx); break;
    case TELA_CADASTRO:         tela_cadastro_desenhar(ctx); break;
    case TELA_MESA:             tela_mesa_desenhar(ctx); break;
    case TELA_FIM_PARTIDA:      tela_fim_desenhar(ctx); break;
    case TELA_CADERNETA:        tela_caderneta_desenhar(ctx); break;
    case TELA_MURAL:            tela_mural_desenhar(ctx); break;
    case TELA_VAR:              tela_var_desenhar(ctx); break;
    case TELA_PARTIDAS_ANTIGAS: tela_partidas_desenhar(ctx); break;
    default: break;
    }
}

static void alternar_tela_cheia(void)
{
    static const int LARGURA_JANELA = LARGURA, ALTURA_JANELA = ALTURA;

    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
        ToggleBorderlessWindowed();
        SetWindowSize(LARGURA_JANELA, ALTURA_JANELA);
    } else {
        ToggleBorderlessWindowed();
    }
}

int main(void)
{
    Contexto *ctx;
    Tela proxima;

    /* o Contexto é grande (partida + estados das telas): fica no heap */
    ctx = calloc(1, sizeof *ctx);
    if (ctx == NULL)
        return 1;

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(LARGURA, ALTURA, "Truco no Bar do Pato");
    SetWindowMinSize(640, 360);
    ChangeDirectory(GetApplicationDirectory());
    SetExitKey(KEY_NULL);  /* Esc é "voltar ao menu", não fechar */
    InitAudioDevice();
    SetTargetFPS(60);
    srand((unsigned) time(NULL)); /* única semente do programa */

    carregar_recursos(&ctx->rec);
    iniciar_caderneta(&ctx->caderneta);
    carregar_caderneta(&ctx->caderneta, ARQUIVO_CADERNETA);

    ctx->tela = ctx->tela_anterior = TELA_MENU;
    abrir_tela(ctx, ctx->tela);

    while (!WindowShouldClose() && ctx->tela != TELA_SAIR) {
        if (IsKeyPressed(KEY_F11))
            alternar_tela_cheia();
        atualizar_escala();

        proxima = atualizar_tela(ctx);
        if (proxima != ctx->tela) {
            ctx->tela_anterior = ctx->tela;
            ctx->tela = proxima;
            abrir_tela(ctx, proxima);
        }
        if (ctx->tela == TELA_SAIR)
            break;

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera_da_janela());
        desenhar_tela(ctx);
        EndMode2D();
        desenhar_faixas_pretas();
        EndDrawing();
    }

    /* fechar o bar: salva, libera tudo e sai */
    salvar_caderneta(&ctx->caderneta, ARQUIVO_CADERNETA);
    tela_var_liberar(ctx);
    fechar_partida_ativa(ctx);
    tela_mural_liberar(ctx);
    tela_partidas_liberar(ctx);
    liberar_caderneta(&ctx->caderneta);
    liberar_recursos(&ctx->rec);
    CloseAudioDevice();
    CloseWindow();
    free(ctx);
    return 0;
}
