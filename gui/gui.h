/*
 * gui.h — 3ª unidade, Pacote 7 (versão 2): contrato entre as telas
 * Responsáveis: Pedro (base), Arthur (main_gui, mesa), Lucca (menu,
 * cadastro, caderneta), Pedro (mural, fim de partida), Rafael (VAR)
 *
 * Cada tela tem 3 funções:
 *   tela_X_abrir(ctx)      ao entrar na tela;
 *   tela_X_atualizar(ctx)  lê mouse/teclado, chama o motor e retorna
 *                          a próxima tela (ou a mesma);
 *   tela_X_desenhar(ctx)   só desenha: recebe const e não muda nada.
 *
 * A janela tem 1280 x 720 LÓGICOS: tudo é desenhado nessas
 * coordenadas e o main_gui escala para o tamanho real da janela (com
 * faixas pretas se a proporção não bater). O mouse é convertido para
 * as mesmas coordenadas, então os cliques acertam em qualquer tamanho.
 *
 * Nenhuma tela chama printf ou scanf: o motor (partida.h) faz o jogo,
 * as telas só desenham e repassam os cliques.
 *
 * Por que widgets próprios e não o raygui: o raygui é "modo imediato"
 * (o mesmo GuiButton desenha E lê o clique), o que quebraria a regra
 * "desenhar não muda nada". Aqui cada widget tem as duas metades:
 * botao_clicado (no atualizar) e desenhar_botao (no desenhar).
 *
 * Depende da raylib e de partida.h, arquivo.h (src/).
 */
#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "partida.h"
#include "arquivo.h"

#define LARGURA 1280
#define ALTURA  720

/* Identidade visual */
#define COR_FELTRO        CLITERAL(Color){ 0x1E, 0x5E, 0x3A, 255 }
#define COR_FELTRO_ESCURO CLITERAL(Color){ 0x0E, 0x33, 0x1E, 255 }
#define COR_MADEIRA       CLITERAL(Color){ 0x6B, 0x3F, 0x23, 255 }
#define COR_MADEIRA_CLARA CLITERAL(Color){ 0x8A, 0x56, 0x33, 255 }
#define COR_PAINEL        CLITERAL(Color){ 0x2B, 0x18, 0x0D, 240 }
#define COR_CREME         CLITERAL(Color){ 0xF4, 0xEB, 0xD0, 255 }
#define COR_CREME_FRACO   CLITERAL(Color){ 0xF4, 0xEB, 0xD0, 150 }
#define COR_PATO          CLITERAL(Color){ 0xF5, 0xC5, 0x18, 255 }
#define COR_PERIGO        CLITERAL(Color){ 0xD7, 0x26, 0x3D, 255 }

#define TAM_TEXTO     20
#define TAM_SUBTITULO 28
#define TAM_TITULO    48

#define DURACAO_ANIM  0.25f   /* segundos, com ease-out */
#define MAX_CONFETES  160

typedef enum {
    TELA_MENU, TELA_CADASTRO, TELA_MESA, TELA_FIM_PARTIDA,
    TELA_CADERNETA, TELA_MURAL, TELA_VAR, TELA_PARTIDAS_ANTIGAS, TELA_SAIR
} Tela;

typedef struct {
    Font fonte, fonte_titulo;
    Texture2D cartas[NUM_NAIPES][NUM_VALORES], verso, logo;
    int tem_sprites;            /* 1 se as cartas vieram de assets/cartas;
                                   0 = desenhadas na hora (desenhar_carta) */
    Sound som_carta, som_embaralhar, som_truco, som_vitoria;
    int tem_som;                /* 0 se o PC não tem saída de áudio */
} Recursos;

/* --- estado de cada tela (fica no Contexto: nada de variável global) --- */

typedef struct {
    char nomes[NUM_JOGADORES][MAX_NOME]; /* [0],[1] = dupla 1 */
    int foco;                            /* caixa sendo digitada */
    int erro_memoria;
} EstadoCadastro;

typedef struct {
    const Jogador *revelado;   /* para quem a cortina já abriu (NULL = fechada) */
    const Jogador *resposta_revelada; /* no pedido: quem responde e já viu */
    const Jogador *ultimo_responde;   /* para notar um aumento (troca quem responde) */
    int onze_revelado;         /* mão de onze: a dupla já viu as 6 cartas */
    /* a última carta jogada desliza da mão até a mesa */
    int anim_ativa;
    Carta anim_carta;
    Vector2 anim_de, anim_para;
    double anim_inicio;
    double pedido_inicio;      /* "TRUCO!" crescendo na tela */
    double faixa_inicio;       /* faixa de fim de rodada / de mão */
    float placar_mostrado[2];  /* sobe animado até o placar de verdade */
    int ultima_mao;            /* para tocar o som de embaralhar */
    const Partida *de_qual;    /* partida a que este estado se refere */
    int id_partida;            /* (ponteiro + número: partida nova zera tudo) */
    EstadoPartida ultimo_estado;
    int aviso_disco;           /* 1 = já avisou falha de gravação */
    double aviso_disco_inicio;
} EstadoMesa;

typedef struct {
    char busca[MAX_NOME];
    const Fregues *ficha;      /* ficha aberta no cartão (NULL = nenhuma) */
    int nao_achou;
    float rolagem;
} EstadoCaderneta;

typedef struct {
    Fregues **vet;             /* mural: ordenado por mãos (mergesort) */
    Fregues **alfabetica;      /* vergonha: ordem A-Z */
    int n;
    int aba;                   /* 0 = mural, 1 = mural da vergonha */
    float rolagem;
} EstadoMural;

typedef struct {
    const NoHist *atual;
    int pos;                   /* 1..tamanho */
    int var_proprio;           /* 1 = carregado do disco: a tela libera */
    int id_antigo;             /* número da partida antiga (0 = a atual) */
} EstadoVar;

typedef struct {
    ResumoPartida *vet;
    int n;
    float rolagem;
    int nao_abriu;             /* id que falhou ao abrir (0 = nenhum) */
} EstadoPartidas;

typedef struct {
    Vector2 pos, vel;
    float giro, vel_giro;
    Color cor;
} Confete;

typedef struct {
    Confete confetes[MAX_CONFETES];
    double inicio;
} EstadoFim;

typedef struct {
    Recursos rec;
    Caderneta caderneta;
    Partida partida;
    int partida_ativa;          /* 1 entre iniciar_partida e encerrar_partida */
    Historico *var_aberto;      /* o VAR que a TELA_VAR esta mostrando */
    Tela tela, tela_anterior;   /* tela_anterior: para o botao Voltar */

    EstadoCadastro cadastro;
    EstadoMesa mesa;
    EstadoCaderneta cad;
    EstadoMural mural;
    EstadoVar var;
    EstadoPartidas antigas;
    EstadoFim fim;
} Contexto;

/* ================================================================== */
/* gui_base.c (Pedro)                                                  */
/* ================================================================== */

/* Carrega fontes, cartas, logo e sons de assets/. O que faltar não
 * derruba o jogo: fonte padrão, cartas desenhadas, sem som.
 * Retorna 1 se as fontes vieram do disco, 0 se usou a padrão. */
int carregar_recursos(Recursos *r);
void liberar_recursos(Recursos *r);
void tocar(const Recursos *r, Sound s);

/* Escala 1280x720 -> janela: calcula a escala e as faixas pretas e
 * converte o mouse para coordenadas lógicas. Chamar 1x por quadro. */
void atualizar_escala(void);
/* Câmera para desenhar em coordenadas lógicas (BeginMode2D). */
Camera2D camera_da_janela(void);
/* Recorte (scissor) de um retângulo lógico, para listas com rolagem. */
void iniciar_recorte(Rectangle r);
/* Pinta de preto o que sobra fora da área 1280x720. */
void desenhar_faixas_pretas(void);

/* Texto com a fonte certa (título >= 40 usa a negrito). */
void texto(const Recursos *r, const char *s, float x, float y, float tam, Color c);
void texto_centro(const Recursos *r, const char *s, float cx, float y, float tam, Color c);
void texto_direita(const Recursos *r, const char *s, float xd, float y, float tam, Color c);
float largura_texto(const Recursos *r, const char *s, float tam);

/* Fundo: feltro verde com vinheta escura nas bordas. */
void desenhar_feltro(void);
/* Painel marrom escuro com borda de madeira. */
void desenhar_painel(Rectangle r);
/* Escurece a tela inteira (fundo de janela modal). */
void escurecer(float quanto);

/* Carta em r (proporção 5:7). virada = mostra o verso.
 * destaque: contorno amarelo-pato. */
void desenhar_carta(const Recursos *rec, Carta c, Rectangle r, int virada, int destaque);
/* Retângulo de uma carta de altura h com centro em (cx, cy). */
Rectangle retangulo_carta(float cx, float cy, float h);

/* Botões: o clique é lido no atualizar e o botão é desenhado no desenhar. */
typedef enum { BOTAO_NORMAL, BOTAO_PRINCIPAL, BOTAO_PERIGO } EstiloBotao;
int mouse_em(Rectangle r);
int botao_clicado(Rectangle r, int ativo);
void desenhar_botao(const Recursos *rec, Rectangle r, const char *rotulo,
                    EstiloBotao estilo, int ativo);

/* Caixa de texto: editar_texto acrescenta o que foi digitado neste
 * quadro (UTF-8, até cap-1 bytes) e trata o Backspace. */
void editar_texto(char *buf, int cap);
void desenhar_caixa_texto(const Recursos *rec, Rectangle r, const char *buf,
                          int foco, const char *dica);
/* Copia buf para destino sem os espaços das pontas. */
void sem_espacos_nas_pontas(const char *buf, char *destino, int cap);

/* Animação: 0..1 desde 'inicio' (segundos), com ease-out. */
float progresso(double inicio, float duracao);
float suavizar(float t);
/* Rolagem com a roda do mouse, presa entre 0 e maximo. */
float rolar(float atual, float maximo, float passo);

/* Botão "Voltar" padrão do canto superior esquerdo. */
Rectangle retangulo_voltar(void);
void desenhar_cabecalho(const Recursos *rec, const char *titulo);

/* ================================================================== */
/* Telas                                                               */
/* ================================================================== */

void tela_menu_abrir(Contexto *ctx);
Tela tela_menu_atualizar(Contexto *ctx);
void tela_menu_desenhar(const Contexto *ctx);

void tela_cadastro_abrir(Contexto *ctx);
Tela tela_cadastro_atualizar(Contexto *ctx);
void tela_cadastro_desenhar(const Contexto *ctx);

void tela_mesa_abrir(Contexto *ctx);
Tela tela_mesa_atualizar(Contexto *ctx);
void tela_mesa_desenhar(const Contexto *ctx);

void tela_fim_abrir(Contexto *ctx);
Tela tela_fim_atualizar(Contexto *ctx);
void tela_fim_desenhar(const Contexto *ctx);

void tela_caderneta_abrir(Contexto *ctx);
Tela tela_caderneta_atualizar(Contexto *ctx);
void tela_caderneta_desenhar(const Contexto *ctx);

void tela_mural_abrir(Contexto *ctx);
Tela tela_mural_atualizar(Contexto *ctx);
void tela_mural_desenhar(const Contexto *ctx);
void tela_mural_liberar(Contexto *ctx);

void tela_var_abrir(Contexto *ctx);
Tela tela_var_atualizar(Contexto *ctx);
void tela_var_desenhar(const Contexto *ctx);
void tela_var_liberar(Contexto *ctx);

void tela_partidas_abrir(Contexto *ctx);
Tela tela_partidas_atualizar(Contexto *ctx);
void tela_partidas_desenhar(const Contexto *ctx);
void tela_partidas_liberar(Contexto *ctx);

/* Termina a partida ativa (se houver) e libera baralho, roda e VAR. */
void fechar_partida_ativa(Contexto *ctx);

#endif /* GUI_H */
