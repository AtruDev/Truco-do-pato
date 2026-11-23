/*
 * arquivo.c — 3ª unidade, Pacote 1: Caderneta em disco
 * Responsável: Rafael
 *
 * salvar_caderneta: percorre a árvore em pré-ordem escrevendo no .tmp
 * e só troca pelo arquivo oficial depois de fechar o .tmp sem erro.
 *
 * carregar_caderneta: lê linha a linha com fgets + sscanf. Linha com
 * defeito (campos faltando, número negativo, nome vazio ou repetido)
 * é pulada e contada; no fim avisa quantas foram puladas.
 * Se o oficial não existir mas o .tmp sim (o programa caiu entre o
 * remove e o rename), lê o .tmp.
 *
 * VAR em disco (historico.txt): as funções gravar_* abrem o arquivo
 * com "a", escrevem um bloco inteiro e fecham. A leitura olha a 1ª
 * letra de cada linha. Um bloco de rodada (R + suas linhas C e E) que
 * vier incompleto ou com número fora da faixa é descartado, e a leitura
 * segue na próxima linha R, P ou F.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arquivo.h"

#define TAM_LINHA_ARQ  128
#define TAM_LINHA_HIST 256   /* a linha P tem a data e 4 nomes */
#define TAM_CAMINHO    260   /* MAX_PATH do Windows */
#define TAM_DATA       17    /* "AAAA-MM-DD HH:MM" + '\0' */

/* Monta "<caminho>.tmp" em destino. Retorna 0 se não couber. */
static int caminho_tmp(const char *caminho, char *destino, size_t tam)
{
    int n = snprintf(destino, tam, "%s.tmp", caminho);
    return n > 0 && (size_t) n < tam;
}

/* Pré-ordem: grava o nó, depois a subárvore esquerda e a direita.
 * Retorna 0 se o fprintf falhar em algum ponto. */
static int gravar_no(FILE *arq, const NoCaderneta *no)
{
    if (no == NULL)
        return 1;
    if (fprintf(arq, "%s;%d;%d;%d;%d;%d\n", no->f.nome, no->f.partidas,
                no->f.maos_ganhas, no->f.trucos_pedidos,
                no->f.trucos_recebidos, no->f.correu) < 0)
        return 0;
    return gravar_no(arq, no->esq) && gravar_no(arq, no->dir);
}

/* Confere uma ficha lida do arquivo: nome não vazio e sem número negativo. */
static int ficha_valida(const Fregues *f)
{
    return f->nome[0] != '\0' && f->partidas >= 0 && f->maos_ganhas >= 0 &&
           f->trucos_pedidos >= 0 && f->trucos_recebidos >= 0 &&
           f->correu >= 0;
}

int salvar_caderneta(const Caderneta *c, const char *caminho)
{
    char tmp[TAM_CAMINHO];
    FILE *arq;
    int ok;

    if (!caminho_tmp(caminho, tmp, sizeof tmp))
        return 0;

    /* 1. escreve tudo no .tmp */
    arq = fopen(tmp, "w");
    if (arq == NULL)
        return 0;
    ok = gravar_no(arq, c->raiz);

    /* 2. fecha e confere (o fclose pode falhar ao descarregar o buffer) */
    if (fclose(arq) != 0)
        ok = 0;
    if (!ok) {
        remove(tmp);
        return 0;   /* o arquivo oficial antigo continua intacto */
    }

    /* 3. no Windows o rename falha se o destino existir */
    remove(caminho);

    /* 4. o .tmp vira o oficial */
    return rename(tmp, caminho) == 0;
}

int carregar_caderneta(Caderneta *c, const char *caminho)
{
    char tmp[TAM_CAMINHO];
    char linha[TAM_LINHA_ARQ];
    FILE *arq;
    Fregues f;
    int carregados = 0, puladas = 0, ch;
    size_t n;

    arq = fopen(caminho, "r");
    if (arq == NULL && caminho_tmp(caminho, tmp, sizeof tmp)) {
        /* recuperação: caiu entre o remove e o rename */
        arq = fopen(tmp, "r");
        if (arq != NULL)
            printf("Aviso: recuperando a caderneta de %s\n", tmp);
    }
    if (arq == NULL)
        return 0;   /* primeira vez: caderneta vazia, não é erro */

    while (fgets(linha, sizeof linha, arq) != NULL) {
        n = strlen(linha);
        if (n > 0 && linha[n - 1] == '\n') {
            linha[--n] = '\0';
        } else if (n == sizeof linha - 1) {
            /* linha comprida demais: descarta o resto e pula */
            while ((ch = fgetc(arq)) != '\n' && ch != EOF)
                ;
            puladas++;
            continue;
        }
        if (n > 0 && linha[n - 1] == '\r')   /* arquivo editado no Windows */
            linha[--n] = '\0';

        memset(&f, 0, sizeof f);
        if (sscanf(linha, "%31[^;];%d;%d;%d;%d;%d", f.nome, &f.partidas,
                   &f.maos_ganhas, &f.trucos_pedidos, &f.trucos_recebidos,
                   &f.correu) != 6 || !ficha_valida(&f)) {
            puladas++;
            continue;
        }
        if (inserir_fregues(c, &f))
            carregados++;
        else
            puladas++;   /* nome repetido (ou falta de memória) */
    }
    fclose(arq);

    if (puladas > 0)
        printf("Aviso: %d linha(s) com defeito na caderneta foram puladas.\n",
               puladas);
    return carregados;
}

/* ================================================================== */
/* VAR em disco (historico.txt)                                        */
/* ================================================================== */

/* Resumo de uma partida para a listagem (só existe dentro deste .c). */
typedef struct {
    int id;
    char data[TAM_DATA];
    char nomes[NUM_JOGADORES][MAX_NOME];  /* ordem da mesa: D1, D2, D1, D2 */
    int terminou, pontos_d1, pontos_d2;
} ResumoPartida;

/* Lê uma linha com fgets e tira o '\n' (e o '\r' do Windows).
 * Linha maior que o buffer: descarta o resto e devolve a linha vazia,
 * que nenhum tipo aceita (conta como linha inválida).
 * Retorna 0 no fim do arquivo. */
static int ler_linha(FILE *arq, char *linha, size_t tam)
{
    size_t n;
    int ch;

    if (fgets(linha, (int) tam, arq) == NULL)
        return 0;
    n = strlen(linha);
    if (n > 0 && linha[n - 1] == '\n') {
        linha[--n] = '\0';
    } else if (n == tam - 1) {
        while ((ch = fgetc(arq)) != '\n' && ch != EOF)
            ;
        linha[0] = '\0';
        return 1;
    }
    if (n > 0 && linha[n - 1] == '\r')
        linha[--n] = '\0';
    return 1;
}

/* Fecha o arquivo e junta o resultado do fclose com o da escrita. */
static int fechar(FILE *arq, int ok)
{
    if (fclose(arq) != 0)
        return 0;
    return ok;
}

/* Cabeçalho de uma rodada: "R;id;mao;rodada;dupla;qtd_cartas;qtd_eventos".
 * Preenche os números em *no (zerado antes) e o id em *id.
 * Retorna 0 se faltar campo ou algum número estiver fora da faixa. */
static int ler_cabecalho_rodada(const char *linha, int *id, NoHist *no)
{
    int mao, rod, dupla, qtd, qtd_ev;

    memset(no, 0, sizeof *no);
    if (sscanf(linha, "R;%d;%d;%d;%d;%d;%d", id, &mao, &rod, &dupla, &qtd,
               &qtd_ev) != 6)
        return 0;
    if (*id < 1 || mao < 1 || rod < 1 || rod > 3 || dupla < RODADA_EMPATADA ||
        dupla > 1 || qtd < 0 || qtd > NUM_JOGADORES || qtd_ev < 0 ||
        qtd_ev > MAX_EVENTOS)
        return 0;
    no->num_mao = mao;
    no->num_rodada = rod;
    no->rodada.dupla_vencedora = dupla;
    no->rodada.qtd = qtd;
    no->qtd_eventos = qtd_ev;
    return 1;
}

/* Linha "C;valor;naipe;nome" -> carta i do nó. Retorna 0 se inválida. */
static int ler_carta(const char *linha, NoHist *no, int i)
{
    int valor, naipe;
    char nome[MAX_NOME];

    if (sscanf(linha, "C;%d;%d;%31[^\n]", &valor, &naipe, nome) != 3)
        return 0;
    if (valor < 0 || valor >= NUM_VALORES || naipe < 0 || naipe >= NUM_NAIPES)
        return 0;
    no->rodada.cartas[i].valor = (Valor) valor;
    no->rodada.cartas[i].naipe = (Naipe) naipe;
    strcpy(no->nomes[i], nome);
    return 1;
}

/* Linha "E;dupla;tipo;nome" -> evento i do nó. Retorna 0 se inválida. */
static int ler_evento(const char *linha, NoHist *no, int i)
{
    int dupla, tipo;
    char nome[MAX_NOME];

    if (sscanf(linha, "E;%d;%d;%31[^\n]", &dupla, &tipo, nome) != 3)
        return 0;
    if (dupla < 0 || dupla > 1 || tipo < PEDIU_TRUCO || tipo > CORREU)
        return 0;
    no->eventos[i] = criar_evento(dupla, nome, (TipoEvento) tipo);
    return 1;
}

int proximo_id_partida(const char *caminho)
{
    FILE *arq = fopen(caminho, "r");
    char linha[TAM_LINHA_HIST];
    char tipo;
    int id, maior = 0;

    if (arq == NULL)
        return 1;
    while (ler_linha(arq, linha, sizeof linha))
        if (sscanf(linha, "%c;%d", &tipo, &id) == 2 &&
            (tipo == 'P' || tipo == 'R' || tipo == 'F') && id > maior)
            maior = id;
    fclose(arq);
    return maior + 1;
}

int gravar_inicio_partida(const char *caminho, int id, const Mesa *m)
{
    char data[TAM_DATA];
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);
    FILE *arq;
    int ok;

    if (t == NULL || strftime(data, sizeof data, "%Y-%m-%d %H:%M", t) == 0)
        strcpy(data, "0000-00-00 00:00");

    arq = fopen(caminho, "a");
    if (arq == NULL)
        return 0;
    ok = fprintf(arq, "P;%d;%s;%s;%s;%s;%s\n", id, data,
                 m->posicoes[0]->nome, m->posicoes[1]->nome,
                 m->posicoes[2]->nome, m->posicoes[3]->nome) > 0;
    return fechar(arq, ok);
}

int gravar_rodada(const char *caminho, int id, const NoHist *no)
{
    FILE *arq;
    int i, ok;

    if (no == NULL)
        return 0;
    arq = fopen(caminho, "a");
    if (arq == NULL)
        return 0;

    ok = fprintf(arq, "R;%d;%d;%d;%d;%d;%d\n", id, no->num_mao,
                 no->num_rodada, no->rodada.dupla_vencedora, no->rodada.qtd,
                 no->qtd_eventos) > 0;
    for (i = 0; ok && i < no->rodada.qtd; i++)
        ok = fprintf(arq, "C;%d;%d;%s\n", (int) no->rodada.cartas[i].valor,
                     (int) no->rodada.cartas[i].naipe, no->nomes[i]) > 0;
    for (i = 0; ok && i < no->qtd_eventos; i++)
        ok = fprintf(arq, "E;%d;%d;%s\n", no->eventos[i].dupla,
                     no->eventos[i].tipo, no->eventos[i].nome) > 0;
    return fechar(arq, ok);
}

int gravar_fim_partida(const char *caminho, int id, int pontos_d1,
                       int pontos_d2, int campea)
{
    FILE *arq = fopen(caminho, "a");
    int ok;

    if (arq == NULL)
        return 0;
    ok = fprintf(arq, "F;%d;%d;%d;%d\n", id, pontos_d1, pontos_d2,
                 campea) > 0;
    return fechar(arq, ok);
}

/* Procura a partida 'id' no vetor de resumos (de trás para a frente,
 * porque o F costuma vir logo depois do P). Retorna o índice ou -1. */
static int achar_resumo(const ResumoPartida *v, int n, int id)
{
    int i;

    for (i = n - 1; i >= 0; i--)
        if (v[i].id == id)
            return i;
    return -1;
}

int listar_partidas_salvas(const char *caminho)
{
    FILE *arq = fopen(caminho, "r");
    char linha[TAM_LINHA_HIST];
    char dupla1[2 * MAX_NOME + 4], dupla2[2 * MAX_NOME + 4];
    ResumoPartida *v = NULL, *maior, r;
    int n = 0, cap = 0, i, id, p1, p2, campea;

    if (arq == NULL)
        return 0;

    /* 1ª etapa: junta as partidas num vetor que cresce conforme precisa */
    while (ler_linha(arq, linha, sizeof linha)) {
        if (linha[0] == 'P') {
            memset(&r, 0, sizeof r);
            if (sscanf(linha, "P;%d;%16[^;];%31[^;];%31[^;];%31[^;];%31[^\n]",
                       &r.id, r.data, r.nomes[0], r.nomes[1], r.nomes[2],
                       r.nomes[3]) != 6 || r.id < 1 ||
                achar_resumo(v, n, r.id) != -1)
                continue; /* linha P com defeito ou repetida */
            if (n == cap) {
                cap = (cap == 0) ? 8 : cap * 2;
                maior = realloc(v, (size_t) cap * sizeof(ResumoPartida));
                if (maior == NULL) {
                    printf("Erro: memoria insuficiente para listar.\n");
                    break;
                }
                v = maior;
            }
            v[n++] = r;
        } else if (linha[0] == 'F') {
            if (sscanf(linha, "F;%d;%d;%d;%d", &id, &p1, &p2, &campea) == 4 &&
                (i = achar_resumo(v, n, id)) != -1) {
                v[i].terminou = 1;
                v[i].pontos_d1 = p1;
                v[i].pontos_d2 = p2;
            }
        }
    }
    fclose(arq);

    /* 2ª etapa: mostra. Dupla 1 = 1º e 3º nome; dupla 2 = 2º e 4º. */
    if (n > 0) {
        printf("\nPartidas salvas no Bar do Pato\n");
        printf("%2s  %-18s%-17s%-17s%s\n", "#", "Data", "Dupla 1", "Dupla 2",
               "Placar");
        for (i = 0; i < n; i++) {
            snprintf(dupla1, sizeof dupla1, "%s e %s", v[i].nomes[0],
                     v[i].nomes[2]);
            snprintf(dupla2, sizeof dupla2, "%s e %s", v[i].nomes[1],
                     v[i].nomes[3]);
            printf("%2d  %-18s%-17s%-17s", v[i].id, v[i].data, dupla1, dupla2);
            if (v[i].terminou)
                printf("%d x %d\n", v[i].pontos_d1, v[i].pontos_d2);
            else
                printf("interrompida\n");
        }
    }
    free(v);
    return n;
}

Historico *carregar_partida(const char *caminho, int id)
{
    FILE *arq;
    char linha[TAM_LINHA_HIST];
    Historico *h;
    NoHist no;
    int achou = 0, em_bloco = 0, cartas = 0, eventos = 0, id_linha = 0;

    arq = fopen(caminho, "r");
    if (arq == NULL)
        return NULL;
    h = criar_historico();
    if (h == NULL) {
        fclose(arq);
        return NULL;
    }
    memset(&no, 0, sizeof no);

    while (ler_linha(arq, linha, sizeof linha)) {
        /* dentro de um bloco R: espera as C e depois as E */
        if (em_bloco) {
            if (linha[0] == 'C' && cartas < no.rodada.qtd &&
                ler_carta(linha, &no, cartas))
                cartas++;
            else if (linha[0] == 'E' && cartas == no.rodada.qtd &&
                     eventos < no.qtd_eventos &&
                     ler_evento(linha, &no, eventos))
                eventos++;
            else
                em_bloco = 0; /* bloco incompleto: a rodada é descartada e
                                 esta linha é lida de novo abaixo */
        }

        /* fora de bloco: só interessam P (a partida existe) e R */
        if (!em_bloco) {
            int id_p;

            if (linha[0] == 'P' && sscanf(linha, "P;%d;", &id_p) == 1 &&
                id_p == id)
                achou = 1;
            else if (linha[0] == 'R' &&
                     ler_cabecalho_rodada(linha, &id_linha, &no)) {
                em_bloco = 1;
                cartas = eventos = 0;
            }
        }

        /* bloco completo: entra na lista se for da partida pedida */
        if (em_bloco && cartas == no.rodada.qtd && eventos == no.qtd_eventos) {
            em_bloco = 0;
            if (id_linha == id) {
                achou = 1;
                if (!registrar_no(h, &no)) {
                    printf("Erro: memoria insuficiente.\n");
                    fclose(arq);
                    liberar_historico(h);
                    return NULL;
                }
            }
        }
    }
    fclose(arq);   /* um bloco aberto no fim do arquivo é descartado */

    if (!achou) {
        liberar_historico(h);
        return NULL;
    }
    return h;
}
