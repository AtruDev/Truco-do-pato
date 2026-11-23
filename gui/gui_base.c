/*
 * gui_base.c — 3ª unidade, Pacote 7: base gráfica comum
 * Responsável: Pedro
 *
 * Paleta, fontes, cartas, logo, sons, botões, caixa de texto, painéis,
 * animações e a escala 1280x720 -> janela. Todas as telas usam daqui.
 *
 * Cartas: se assets/cartas/ tiver as 40 imagens (<valor>_<naipe>.png,
 * ex. "Q_copas.png", "4_ouros.png") e "verso.png", elas são usadas.
 * Senão, a carta é desenhada na hora com formas e a fonte DejaVu
 * (que tem os naipes), sem precisar de arquivo nenhum.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gui.h"

#define ARQ_FONTE        "assets/fonte/DejaVuSans.ttf"
#define ARQ_FONTE_TITULO "assets/fonte/DejaVuSans-Bold.ttf"
#define TAM_ATLAS        40   /* fonte carregada grande e reduzida no desenho */
#define TAM_ATLAS_TITULO 80

/* ------------------------------------------------------------------ */
/* Recursos                                                            */
/* ------------------------------------------------------------------ */

/* ASCII, Latin-1 (acentos) e os símbolos que as telas usam. */
static int montar_codepoints(int cp[], int max)
{
    static const int extras[] = {
        0x2660, 0x2663, 0x2665, 0x2666,   /* ♠ ♣ ♥ ♦ */
        0x25C0, 0x25B6,                   /* ◀ ▶ */
        0x2014, 0x2013, 0x2026, 0x2022,   /* — – … • */
        0x2190, 0x2192, 0x00D7            /* ← → × */
    };
    int n = 0, c;
    size_t i;

    for (c = 32; c <= 126 && n < max; c++)
        cp[n++] = c;
    for (c = 160; c <= 255 && n < max; c++)
        cp[n++] = c;
    for (i = 0; i < sizeof extras / sizeof extras[0] && n < max; i++)
        cp[n++] = extras[i];
    return n;
}

static Font carregar_fonte(const char *caminho, int tamanho)
{
    int cp[256];
    int n = montar_codepoints(cp, 256);
    Font f = LoadFontEx(caminho, tamanho, cp, n);

    if (f.texture.id == 0)
        return GetFontDefault();
    GenTextureMipmaps(&f.texture);
    SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
    return f;
}

/* Logo do Bar do Pato: um pato amarelo de chapéu, desenhado numa
 * imagem (serve também de ícone da janela). */
static Image imagem_logo(void)
{
    Image img = GenImageColor(256, 256, BLANK);
    Color laranja = { 0xF2, 0x8C, 0x28, 255 };

    ImageDrawCircle(&img, 128, 160, 84, COR_MADEIRA);           /* fundo */
    ImageDrawCircle(&img, 128, 160, 76, COR_PATO);              /* corpo */
    ImageDrawCircle(&img, 128, 104, 52, COR_PATO);              /* cabeça */
    ImageDrawTriangle(&img, (Vector2){ 150, 100 }, (Vector2){ 150, 128 },
                      (Vector2){ 212, 114 }, laranja);          /* bico */
    ImageDrawCircle(&img, 142, 92, 9, COR_PAINEL);              /* olho */
    ImageDrawCircle(&img, 145, 89, 3, WHITE);
    ImageDrawRectangle(&img, 72, 52, 112, 12, COR_PAINEL);      /* aba */
    ImageDrawRectangle(&img, 92, 14, 72, 42, COR_PAINEL);       /* chapéu */
    ImageDrawRectangle(&img, 92, 42, 72, 8, COR_PERIGO);        /* fita */
    return img;
}

static void carregar_cartas(Recursos *r)
{
    char caminho[96];
    int n, v, ok = 1;

    for (n = 0; n < NUM_NAIPES; n++)
        for (v = 0; v < NUM_VALORES; v++) {
            snprintf(caminho, sizeof caminho, "assets/cartas/%s_%s.png",
                     texto_valor((Valor) v), texto_naipe((Naipe) n));
            r->cartas[n][v] = (Texture2D){ 0 };
            if (ok && FileExists(caminho)) {
                r->cartas[n][v] = LoadTexture(caminho);
                SetTextureFilter(r->cartas[n][v], TEXTURE_FILTER_BILINEAR);
            }
            if (r->cartas[n][v].id == 0)
                ok = 0;
        }
    r->verso = (Texture2D){ 0 };
    if (ok && FileExists("assets/cartas/verso.png"))
        r->verso = LoadTexture("assets/cartas/verso.png");
    r->tem_sprites = ok && r->verso.id != 0;
}

static Sound carregar_som(const Recursos *r, const char *caminho)
{
    if (!r->tem_som || !FileExists(caminho))
        return (Sound){ 0 };
    return LoadSound(caminho);
}

int carregar_recursos(Recursos *r)
{
    Image logo;
    int fontes_ok;

    memset(r, 0, sizeof *r);
    r->fonte = carregar_fonte(ARQ_FONTE, TAM_ATLAS);
    r->fonte_titulo = carregar_fonte(ARQ_FONTE_TITULO, TAM_ATLAS_TITULO);
    fontes_ok = r->fonte.texture.id != GetFontDefault().texture.id;

    logo = imagem_logo();
    SetWindowIcon(logo);
    r->logo = LoadTextureFromImage(logo);
    SetTextureFilter(r->logo, TEXTURE_FILTER_BILINEAR);
    UnloadImage(logo);

    carregar_cartas(r);

    r->tem_som = IsAudioDeviceReady();
    r->som_carta = carregar_som(r, "assets/sons/carta.wav");
    r->som_embaralhar = carregar_som(r, "assets/sons/embaralhar.wav");
    r->som_truco = carregar_som(r, "assets/sons/truco.wav");
    r->som_vitoria = carregar_som(r, "assets/sons/vitoria.wav");
    return fontes_ok;
}

static void liberar_fonte(Font f)
{
    if (f.texture.id != GetFontDefault().texture.id)
        UnloadFont(f);
}

void liberar_recursos(Recursos *r)
{
    int n, v;

    liberar_fonte(r->fonte);
    liberar_fonte(r->fonte_titulo);
    UnloadTexture(r->logo);
    for (n = 0; n < NUM_NAIPES; n++)
        for (v = 0; v < NUM_VALORES; v++)
            if (r->cartas[n][v].id != 0)
                UnloadTexture(r->cartas[n][v]);
    if (r->verso.id != 0)
        UnloadTexture(r->verso);
    if (r->tem_som) {
        UnloadSound(r->som_carta);
        UnloadSound(r->som_embaralhar);
        UnloadSound(r->som_truco);
        UnloadSound(r->som_vitoria);
    }
}

void tocar(const Recursos *r, Sound s)
{
    if (r->tem_som && s.frameCount > 0)
        PlaySound(s);
}

/* ------------------------------------------------------------------ */
/* Escala da janela                                                    */
/* ------------------------------------------------------------------ */

/* Escala e deslocamento da área lógica dentro da janela de agora. */
static void medir(float *escala, float *ox, float *oy)
{
    float w = (float) GetScreenWidth(), h = (float) GetScreenHeight();
    float s = fminf(w / LARGURA, h / ALTURA);

    if (s <= 0.0f)
        s = 1.0f;
    *escala = s;
    *ox = (w - LARGURA * s) / 2.0f;
    *oy = (h - ALTURA * s) / 2.0f;
}

void atualizar_escala(void)
{
    float s, ox, oy;

    medir(&s, &ox, &oy);
    SetMouseOffset((int) -ox, (int) -oy);
    SetMouseScale(1.0f / s, 1.0f / s);
}

Camera2D camera_da_janela(void)
{
    Camera2D cam = { 0 };
    float s, ox, oy;

    medir(&s, &ox, &oy);
    cam.offset = (Vector2){ ox, oy };
    cam.zoom = s;
    return cam;
}

void iniciar_recorte(Rectangle r)
{
    float s, ox, oy;

    medir(&s, &ox, &oy);
    BeginScissorMode((int) (r.x * s + ox), (int) (r.y * s + oy),
                     (int) (r.width * s), (int) (r.height * s));
}

void desenhar_faixas_pretas(void)
{
    float s, ox, oy;
    int w = GetScreenWidth(), h = GetScreenHeight();

    medir(&s, &ox, &oy);
    if (ox > 0) {
        DrawRectangle(0, 0, (int) ceilf(ox), h, BLACK);
        DrawRectangle(w - (int) ceilf(ox), 0, (int) ceilf(ox) + 1, h, BLACK);
    }
    if (oy > 0) {
        DrawRectangle(0, 0, w, (int) ceilf(oy), BLACK);
        DrawRectangle(0, h - (int) ceilf(oy), w, (int) ceilf(oy) + 1, BLACK);
    }
}

/* ------------------------------------------------------------------ */
/* Texto                                                               */
/* ------------------------------------------------------------------ */

static Font fonte_para(const Recursos *r, float tam)
{
    return tam >= 40.0f ? r->fonte_titulo : r->fonte;
}

float largura_texto(const Recursos *r, const char *s, float tam)
{
    return MeasureTextEx(fonte_para(r, tam), s, tam, 0.0f).x;
}

void texto(const Recursos *r, const char *s, float x, float y, float tam, Color c)
{
    DrawTextEx(fonte_para(r, tam), s, (Vector2){ roundf(x), roundf(y) }, tam,
               0.0f, c);
}

void texto_centro(const Recursos *r, const char *s, float cx, float y, float tam,
                  Color c)
{
    texto(r, s, cx - largura_texto(r, s, tam) / 2.0f, y, tam, c);
}

void texto_direita(const Recursos *r, const char *s, float xd, float y, float tam,
                   Color c)
{
    texto(r, s, xd - largura_texto(r, s, tam), y, tam, c);
}

/* ------------------------------------------------------------------ */
/* Fundos e painéis                                                    */
/* ------------------------------------------------------------------ */

void desenhar_feltro(void)
{
    Color borda = { 0, 0, 0, 120 }, nada = { 0, 0, 0, 0 };

    DrawRectangle(0, 0, LARGURA, ALTURA, COR_FELTRO);
    DrawCircleGradient((Vector2){ LARGURA / 2.0f, ALTURA / 2.0f }, 520.0f,
                       (Color){ 0x2A, 0x74, 0x4A, 90 }, nada);
    /* vinheta: as bordas escurecem */
    DrawRectangleGradientV(0, 0, LARGURA, 140, borda, nada);
    DrawRectangleGradientV(0, ALTURA - 140, LARGURA, 140, nada, borda);
    DrawRectangleGradientH(0, 0, 200, ALTURA, borda, nada);
    DrawRectangleGradientH(LARGURA - 200, 0, 200, ALTURA, nada, borda);
}

void desenhar_painel(Rectangle r)
{
    DrawRectangleRounded((Rectangle){ r.x + 4, r.y + 6, r.width, r.height },
                         0.06f, 8, (Color){ 0, 0, 0, 90 });
    DrawRectangleRounded(r, 0.06f, 8, COR_PAINEL);
    DrawRectangleRoundedLinesEx(r, 0.06f, 8, 3.0f, COR_MADEIRA_CLARA);
}

void escurecer(float quanto)
{
    DrawRectangle(0, 0, LARGURA, ALTURA, (Color){ 0, 0, 0, (unsigned char) (quanto * 255) });
}

/* ------------------------------------------------------------------ */
/* Cartas                                                              */
/* ------------------------------------------------------------------ */

Rectangle retangulo_carta(float cx, float cy, float h)
{
    float w = h * 5.0f / 7.0f;
    return (Rectangle){ cx - w / 2.0f, cy - h / 2.0f, w, h };
}

static const char *simbolo_naipe(Naipe n)
{
    switch (n) {
    case OUROS:   return "\xE2\x99\xA6"; /* ♦ */
    case ESPADAS: return "\xE2\x99\xA0"; /* ♠ */
    case COPAS:   return "\xE2\x99\xA5"; /* ♥ */
    default:      return "\xE2\x99\xA3"; /* ♣ */
    }
}

static void desenhar_verso(Rectangle r)
{
    Color vermelho = { 0x9E, 0x1B, 0x2C, 255 };
    Rectangle dentro = { r.x + r.width * 0.09f, r.y + r.width * 0.09f,
                         r.width * 0.82f, r.height - r.width * 0.18f };
    float passo = r.width / 5.0f;
    float x;

    DrawRectangleRounded(r, 0.12f, 6, COR_CREME);
    DrawRectangleRounded(dentro, 0.10f, 6, vermelho);
    BeginBlendMode(BLEND_ALPHA);
    for (x = dentro.x - dentro.height; x < dentro.x + dentro.width; x += passo) {
        Vector2 a = { fmaxf(x, dentro.x), 0 }, b;
        /* diagonais clipadas na área de dentro (aproximação simples) */
        a.y = dentro.y + (a.x - x);
        b.x = fminf(x + dentro.height, dentro.x + dentro.width);
        b.y = dentro.y + (b.x - x);
        if (a.y < dentro.y + dentro.height)
            DrawLineEx(a, b, 1.5f, (Color){ 0xF5, 0xC5, 0x18, 70 });
    }
    EndBlendMode();
    DrawCircleV((Vector2){ r.x + r.width / 2, r.y + r.height / 2 }, r.width * 0.17f,
                COR_PATO);
    DrawCircleV((Vector2){ r.x + r.width * 0.56f, r.y + r.height * 0.47f },
                r.width * 0.03f, COR_PAINEL);
}

void desenhar_carta(const Recursos *rec, Carta c, Rectangle r, int virada, int destaque)
{
    Color tinta;
    char canto[8];
    const char *naipe;
    float t;

    /* sombra */
    DrawRectangleRounded((Rectangle){ r.x + 3, r.y + 4, r.width, r.height }, 0.12f,
                         6, (Color){ 0, 0, 0, 80 });

    if (rec->tem_sprites) {
        Texture2D tex = virada ? rec->verso : rec->cartas[c.naipe][c.valor];
        DrawTexturePro(tex, (Rectangle){ 0, 0, (float) tex.width, (float) tex.height },
                       r, (Vector2){ 0, 0 }, 0.0f, WHITE);
    } else if (virada) {
        desenhar_verso(r);
    } else {
        tinta = (c.naipe == OUROS || c.naipe == COPAS)
                    ? (Color){ 0xC4, 0x1E, 0x32, 255 }
                    : (Color){ 0x1C, 0x1C, 0x22, 255 };
        naipe = simbolo_naipe(c.naipe);
        t = r.height;

        DrawRectangleRounded(r, 0.12f, 6, (Color){ 0xFF, 0xFC, 0xF2, 255 });
        DrawRectangleRoundedLinesEx(r, 0.12f, 6, 1.0f, (Color){ 0, 0, 0, 60 });

        /* canto de cima e de baixo: valor e naipe */
        snprintf(canto, sizeof canto, "%s", texto_valor(c.valor));
        texto(rec, canto, r.x + t * 0.06f, r.y + t * 0.03f, t * 0.20f, tinta);
        texto(rec, naipe, r.x + t * 0.065f, r.y + t * 0.22f, t * 0.15f, tinta);
        texto_direita(rec, canto, r.x + r.width - t * 0.06f,
                      r.y + r.height - t * 0.24f, t * 0.20f, tinta);

        /* centro: naipe grande (figuras ganham a letra grande também) */
        if (c.valor == DAMA || c.valor == VALETE || c.valor == REI) {
            DrawRectangleRoundedLinesEx(
                (Rectangle){ r.x + r.width * 0.24f, r.y + r.height * 0.2f,
                             r.width * 0.52f, r.height * 0.6f },
                0.15f, 4, 1.5f, tinta);
            texto_centro(rec, canto, r.x + r.width / 2, r.y + t * 0.26f, t * 0.32f,
                         tinta);
            texto_centro(rec, naipe, r.x + r.width / 2, r.y + t * 0.55f, t * 0.18f,
                         tinta);
        } else {
            texto_centro(rec, naipe, r.x + r.width / 2, r.y + t * 0.28f, t * 0.42f,
                         tinta);
        }
    }

    if (destaque)
        DrawRectangleRoundedLinesEx((Rectangle){ r.x - 4, r.y - 4, r.width + 8,
                                                 r.height + 8 },
                                    0.14f, 6, 4.0f, COR_PATO);
}

/* ------------------------------------------------------------------ */
/* Botões                                                              */
/* ------------------------------------------------------------------ */

int mouse_em(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r);
}

int botao_clicado(Rectangle r, int ativo)
{
    return ativo && mouse_em(r) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void desenhar_botao(const Recursos *rec, Rectangle r, const char *rotulo,
                    EstiloBotao estilo, int ativo)
{
    Color fundo, letra = COR_CREME;
    int sobre = ativo && mouse_em(r);
    float tam = r.height >= 64 ? TAM_SUBTITULO : TAM_TEXTO;

    switch (estilo) {
    case BOTAO_PRINCIPAL:
        fundo = COR_PATO;
        letra = COR_PAINEL;
        break;
    case BOTAO_PERIGO:
        fundo = COR_PERIGO;
        break;
    default:
        fundo = COR_MADEIRA;
        break;
    }
    if (!ativo) {
        fundo = (Color){ 90, 80, 70, 200 };
        letra = (Color){ 190, 180, 160, 200 };
    } else if (sobre) {
        fundo = ColorBrightness(fundo, 0.15f);
    }
    if (sobre && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        r.y += 2;

    DrawRectangleRounded((Rectangle){ r.x, r.y + 4, r.width, r.height }, 0.3f, 8,
                         (Color){ 0, 0, 0, 90 });
    DrawRectangleRounded(r, 0.3f, 8, fundo);
    if (sobre)
        DrawRectangleRoundedLinesEx(r, 0.3f, 8, 2.0f, COR_CREME);
    texto_centro(rec, rotulo, r.x + r.width / 2, r.y + (r.height - tam) / 2 - 1, tam,
                 letra);
}

/* ------------------------------------------------------------------ */
/* Caixa de texto                                                      */
/* ------------------------------------------------------------------ */

void editar_texto(char *buf, int cap)
{
    int cp, n, len = (int) strlen(buf);
    const char *utf8;

    while ((cp = GetCharPressed()) > 0) {
        if (cp < 32)
            continue;
        utf8 = CodepointToUTF8(cp, &n);
        if (len + n < cap) {
            memcpy(buf + len, utf8, (size_t) n);
            len += n;
            buf[len] = '\0';
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        /* apaga um caractere inteiro (UTF-8 pode ter vários bytes) */
        while (len > 0 && ((unsigned char) buf[len - 1] & 0xC0) == 0x80)
            len--;
        if (len > 0)
            len--;
        buf[len] = '\0';
    }
}

void desenhar_caixa_texto(const Recursos *rec, Rectangle r, const char *buf,
                          int foco, const char *dica)
{
    float x;

    DrawRectangleRounded(r, 0.2f, 6, (Color){ 0xFF, 0xFC, 0xF2, 255 });
    DrawRectangleRoundedLinesEx(r, 0.2f, 6, foco ? 3.0f : 1.5f,
                                foco ? COR_PATO : COR_MADEIRA);
    if (buf[0] == '\0' && dica != NULL)
        texto(rec, dica, r.x + 12, r.y + (r.height - TAM_TEXTO) / 2,
              TAM_TEXTO, (Color){ 120, 110, 100, 255 });
    else
        texto(rec, buf, r.x + 12, r.y + (r.height - TAM_TEXTO) / 2, TAM_TEXTO,
              COR_PAINEL);
    if (foco && fmod(GetTime(), 1.0) < 0.55) {
        x = r.x + 13 + largura_texto(rec, buf, TAM_TEXTO);
        DrawRectangle((int) x, (int) (r.y + 9), 2, (int) (r.height - 18), COR_PAINEL);
    }
}

void sem_espacos_nas_pontas(const char *buf, char *destino, int cap)
{
    int ini = 0, fim = (int) strlen(buf), n;

    while (buf[ini] == ' ' || buf[ini] == '\t')
        ini++;
    while (fim > ini && (buf[fim - 1] == ' ' || buf[fim - 1] == '\t'))
        fim--;
    n = fim - ini;
    if (n > cap - 1)
        n = cap - 1;
    memcpy(destino, buf + ini, (size_t) n);
    destino[n] = '\0';
}

/* ------------------------------------------------------------------ */
/* Animação e rolagem                                                  */
/* ------------------------------------------------------------------ */

float suavizar(float t)
{
    float u = 1.0f - t;
    return 1.0f - u * u * u; /* ease-out cúbico */
}

float progresso(double inicio, float duracao)
{
    float t = (float) ((GetTime() - inicio) / duracao);
    return suavizar(t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t));
}

float rolar(float atual, float maximo, float passo)
{
    atual -= GetMouseWheelMove() * passo;
    if (atual > maximo)
        atual = maximo;
    if (atual < 0.0f)
        atual = 0.0f;
    return atual;
}

/* ------------------------------------------------------------------ */
/* Cabeçalho padrão                                                    */
/* ------------------------------------------------------------------ */

Rectangle retangulo_voltar(void)
{
    return (Rectangle){ 20, 14, 140, 44 };
}

void desenhar_cabecalho(const Recursos *rec, const char *titulo)
{
    DrawRectangle(0, 0, LARGURA, 72, COR_MADEIRA);
    DrawRectangle(0, 72, LARGURA, 4, COR_PAINEL);
    texto_centro(rec, titulo, LARGURA / 2.0f, 16, 40, COR_CREME);
    desenhar_botao(rec, retangulo_voltar(), "\xE2\x86\x90 Voltar", BOTAO_NORMAL, 1);
}
