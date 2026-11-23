/*
 * caderneta.h — 3ª unidade, Pacote 2: Caderneta de fregueses
 * Responsável: Lucca
 *
 * Uma ficha (Fregues) para cada pessoa que já sentou na mesa, com os
 * contadores acumulados de todas as partidas. As fichas ficam numa
 * ÁRVORE BINÁRIA DE BUSCA ordenada pelo nome (sem diferenciar
 * maiúsculas), o que dá:
 *   - busca O(log n) no caso médio (O(n) no pior caso, árvore torta);
 *   - listagem em ordem alfabética pelo percurso em ordem.
 * (Uma tabela hash buscaria rápido, mas não lista em ordem.)
 *
 * Não existe remoção: freguês nunca sai da caderneta. Por isso um
 * Fregues * devolvido aqui continua válido até liberar_caderneta(), e
 * o main guarda esse ponteiro em cada jogador (Jogador.ficha).
 *
 * Depende de jogador.h (MAX_NOME).
 */
#ifndef CADERNETA_H
#define CADERNETA_H

#include "jogador.h"   /* MAX_NOME */

typedef struct Fregues {
    char nome[MAX_NOME];   /* chave da busca (MAX_NOME vem de jogador.h) */
    int partidas;          /* partidas em que sentou na mesa */
    int maos_ganhas;       /* maos que a dupla dele levou */
    int trucos_pedidos;    /* pedidos que ELE fez: truco, seis ou doze */
    int trucos_recebidos;  /* pedidos que ELE respondeu */
    int correu;            /* vezes que ELE correu de um pedido */
} Fregues;

typedef struct NoCaderneta {
    Fregues f;
    struct NoCaderneta *esq, *dir;   /* esq: nomes menores, dir: maiores */
} NoCaderneta;

typedef struct {
    NoCaderneta *raiz;
    int quantidade;                  /* quantos fregueses tem */
} Caderneta;

/* Compara dois nomes sem diferenciar maiusculas (como strcmp:
 * <0, 0 ou >0). Todo modulo compara nomes por aqui. */
int comparar_nomes(const char *a, const char *b);

/* Caderneta vazia. */
void iniciar_caderneta(Caderneta *c);

/* Procura pelo nome. Retorna a ficha ou NULL se nao achar. */
Fregues *buscar_fregues(const Caderneta *c, const char *nome);

/* Procura pelo nome; se nao achar, cria a ficha zerada e insere.
 * *novo = 1 se foi criada agora, 0 se ja existia.
 * Retorna NULL so se faltar memoria. */
Fregues *buscar_ou_cadastrar(Caderneta *c, const char *nome, int *novo);

/* Insere uma COPIA de f, com os contadores como estao (o arquivo usa
 * para recarregar). Retorna 1 se inseriu, 0 se o nome ja existe ou
 * faltou memoria. */
int inserir_fregues(Caderneta *c, const Fregues *f);

/* Mostra a caderneta inteira em ordem alfabetica (percurso em ordem). */
void listar_caderneta(const Caderneta *c);

/* Preenche vet[] com ponteiros para as fichas, em ordem alfabetica.
 * Grava no maximo 'max'. Retorna quantos gravou. */
int caderneta_para_vetor(const Caderneta *c, Fregues *vet[], int max);

/* Altura da arvore (vazia = 0). Usada no teste e no relatorio. */
int altura_caderneta(const Caderneta *c);

/* Libera todos os nos. A caderneta volta a ficar vazia. */
void liberar_caderneta(Caderneta *c);

#endif /* CADERNETA_H */
