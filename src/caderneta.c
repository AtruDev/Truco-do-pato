/*
 * caderneta.c — 3ª unidade, Pacote 2: Caderneta de fregueses
 * Responsável: Lucca
 *
 * Árvore binária de busca sem balanceamento. Cada nó tem a ficha
 * (por valor) e os filhos: à esquerda os nomes menores, à direita os
 * maiores, sempre comparando com comparar_nomes().
 *
 * A busca e a inserção são iterativas (descem com um while); a
 * listagem, a cópia para vetor, a altura e a liberação são recursivas
 * (funções static que recebem um nó).
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "caderneta.h"

/* ------------------------------------------------------------------ */
/* Funções internas                                                    */
/* ------------------------------------------------------------------ */

/* Cria um nó com uma cópia de f e sem filhos. Retorna NULL se faltar
 * memória. O nome é copiado com terminador garantido. */
static NoCaderneta *criar_no(const Fregues *f)
{
    NoCaderneta *no = malloc(sizeof(NoCaderneta));

    if (no == NULL)
        return NULL;
    no->f = *f;
    no->f.nome[MAX_NOME - 1] = '\0';
    no->esq = no->dir = NULL;
    return no;
}

/* Desce a árvore como na busca e devolve o endereço do ponteiro onde
 * o nome está (ou onde ele deveria ser pendurado, se *resultado == NULL).
 * Usado pela inserção para não repetir a descida. */
static NoCaderneta **lugar_do_nome(NoCaderneta **raiz, const char *nome)
{
    NoCaderneta **p = raiz;
    int cmp;

    while (*p != NULL) {
        cmp = comparar_nomes(nome, (*p)->f.nome);
        if (cmp == 0)
            break;
        p = (cmp < 0) ? &(*p)->esq : &(*p)->dir;
    }
    return p;
}

/* Percurso em ordem (esquerda, nó, direita): imprime uma linha por ficha. */
static void listar_no(const NoCaderneta *no)
{
    if (no == NULL)
        return;
    listar_no(no->esq);
    printf("%-20s%8d%6d%7d%9d%8d\n", no->f.nome, no->f.partidas,
           no->f.maos_ganhas, no->f.trucos_pedidos, no->f.trucos_recebidos,
           no->f.correu);
    listar_no(no->dir);
}

/* Percurso em ordem gravando ponteiros em vet[*qtd], sem passar de max. */
static void no_para_vetor(NoCaderneta *no, Fregues *vet[], int max, int *qtd)
{
    if (no == NULL || *qtd >= max)
        return;
    no_para_vetor(no->esq, vet, max, qtd);
    if (*qtd < max)
        vet[(*qtd)++] = &no->f;
    no_para_vetor(no->dir, vet, max, qtd);
}

/* Altura de uma subárvore: 0 se vazia, senão 1 + a maior das duas. */
static int altura_no(const NoCaderneta *no)
{
    int he, hd;

    if (no == NULL)
        return 0;
    he = altura_no(no->esq);
    hd = altura_no(no->dir);
    return 1 + (he > hd ? he : hd);
}

/* Pós-ordem: libera os filhos antes do pai. */
static void liberar_no(NoCaderneta *no)
{
    if (no == NULL)
        return;
    liberar_no(no->esq);
    liberar_no(no->dir);
    free(no);
}

/* ------------------------------------------------------------------ */
/* Funções públicas                                                    */
/* ------------------------------------------------------------------ */

/* Caractere por caractere com tolower. O cast para unsigned char evita
 * comportamento indefinido com bytes acima de 127 (acentos). */
int comparar_nomes(const char *a, const char *b)
{
    int ca, cb;

    for (;;) {
        ca = tolower((unsigned char) *a);
        cb = tolower((unsigned char) *b);
        if (ca != cb || ca == '\0')
            return ca - cb;
        a++;
        b++;
    }
}

void iniciar_caderneta(Caderneta *c)
{
    c->raiz = NULL;
    c->quantidade = 0;
}

/* Busca iterativa: desce pela esquerda ou pela direita até achar o
 * nome ou cair num NULL. O(altura) = O(log n) no caso médio. */
Fregues *buscar_fregues(const Caderneta *c, const char *nome)
{
    NoCaderneta *no = c->raiz;
    int cmp;

    while (no != NULL) {
        cmp = comparar_nomes(nome, no->f.nome);
        if (cmp == 0)
            return &no->f;
        no = (cmp < 0) ? no->esq : no->dir;
    }
    return NULL;
}

Fregues *buscar_ou_cadastrar(Caderneta *c, const char *nome, int *novo)
{
    NoCaderneta **lugar = lugar_do_nome(&c->raiz, nome);
    Fregues zerada;

    if (*lugar != NULL) {          /* já existe */
        *novo = 0;
        return &(*lugar)->f;
    }

    /* ficha nova: nome copiado com segurança e 5 contadores em 0 */
    memset(&zerada, 0, sizeof zerada);
    strncpy(zerada.nome, nome, MAX_NOME - 1);
    zerada.nome[MAX_NOME - 1] = '\0';

    *lugar = criar_no(&zerada);
    if (*lugar == NULL) {
        *novo = 0;
        return NULL;               /* falta de memória */
    }
    c->quantidade++;
    *novo = 1;
    return &(*lugar)->f;
}

int inserir_fregues(Caderneta *c, const Fregues *f)
{
    NoCaderneta **lugar = lugar_do_nome(&c->raiz, f->nome);

    if (*lugar != NULL)
        return 0;                  /* nome repetido: não insere */
    *lugar = criar_no(f);
    if (*lugar == NULL)
        return 0;
    c->quantidade++;
    return 1;
}

void listar_caderneta(const Caderneta *c)
{
    if (c->raiz == NULL) {
        printf("A caderneta esta vazia.\n");
        return;
    }
    printf("Caderneta do Bar do Pato (%d fregues%s)\n", c->quantidade,
           c->quantidade == 1 ? "" : "es");
    printf("%-20s%8s%6s%7s%9s%8s\n", "Nome", "Partidas", "Maos", "Pediu",
           "Recebeu", "Correu");
    listar_no(c->raiz);
}

int caderneta_para_vetor(const Caderneta *c, Fregues *vet[], int max)
{
    int qtd = 0;

    if (max <= 0)
        return 0;
    no_para_vetor(c->raiz, vet, max, &qtd);
    return qtd;
}

int altura_caderneta(const Caderneta *c)
{
    return altura_no(c->raiz);
}

void liberar_caderneta(Caderneta *c)
{
    liberar_no(c->raiz);
    c->raiz = NULL;
    c->quantidade = 0;
}
