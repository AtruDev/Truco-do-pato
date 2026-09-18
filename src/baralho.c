/*
 * baralho.c — Módulo 2: Baralho
 * Responsável: Rafael
 *
 * O baralho é uma PILHA, usando o TAD estruturas/pilha.h. A carta é
 * guardada na pilha como um código inteiro (o tp_item do TAD é int):
 *
 *     codigo = naipe * NUM_VALORES + valor      (0..39)
 *
 * A única forma de tirar carta é puxar_do_topo(), que chama removePilha().
 *
 * pilha.h traz as funções definidas no próprio header, então ele só pode
 * ser incluído neste arquivo (senão o linker acusa definição duplicada).
 * Por isso Baralho é opaco para os outros módulos.
 */
#include <stdlib.h>
#include "baralho.h"
#include "pilha.h"

struct Baralho {
    tp_pilha pilha; /* códigos das cartas; o topo é a próxima a sair */
};

/* Carta -> código inteiro guardado na pilha. */
static int codificar(Carta c)
{
    return c.naipe * NUM_VALORES + c.valor;
}

/* Código inteiro da pilha -> Carta. */
static Carta decodificar(int codigo)
{
    Carta c;

    c.naipe = (Naipe) (codigo / NUM_VALORES);
    c.valor = (Valor) (codigo % NUM_VALORES);
    return c;
}

/* Esvazia a pilha e empilha as 40 cartas na ordem do vetor codigos[].
 * A última carta do vetor fica no topo. */
static void empilhar_cartas(Baralho *b, const int codigos[TAM_BARALHO])
{
    int i;

    inicializarPilha(&b->pilha);
    for (i = 0; i < TAM_BARALHO; i++)
        inserePilha(&b->pilha, codigos[i]);
}

/* Gera as 40 cartas em ordem (naipe por naipe, do 4 ao 3) e empilha.
 * Os enums Naipe e Valor já não têm 8, 9 e 10, então bastam dois laços. */
Baralho *criar_baralho(void)
{
    Baralho *b = malloc(sizeof(Baralho));
    int codigos[TAM_BARALHO];
    int n, v, i = 0;

    if (b == NULL)
        return NULL;

    for (n = 0; n < NUM_NAIPES; n++) {
        for (v = 0; v < NUM_VALORES; v++) {
            Carta c;
            c.naipe = (Naipe) n;
            c.valor = (Valor) v;
            codigos[i++] = codificar(c);
        }
    }
    empilhar_cartas(b, codigos);
    return b;
}

/* Recolhe o baralho: as 40 cartas voltam, mesmo as que já tinham saído.
 * Embaralha com Fisher-Yates num vetor auxiliar (percorre de trás para
 * frente trocando cada posição i com uma j sorteada entre 0 e i; toda
 * permutação tem a mesma chance, em O(n)) e depois empilha o resultado.
 * É o único ponto de aleatoriedade do jogo (srand é chamado no main). */
void embaralhar(Baralho *b)
{
    int codigos[TAM_BARALHO];
    int i, j, tmp;

    for (i = 0; i < TAM_BARALHO; i++)
        codigos[i] = i; /* os códigos 0..39 são exatamente as 40 cartas */

    for (i = TAM_BARALHO - 1; i > 0; i--) {
        j = rand() % (i + 1);
        tmp = codigos[i];
        codigos[i] = codigos[j];
        codigos[j] = tmp;
    }
    empilhar_cartas(b, codigos);
}

/* Desempilha a carta do topo. Retorna 0, sem mexer em *saida,
 * se o baralho já acabou. */
int puxar_do_topo(Baralho *b, Carta *saida)
{
    int codigo;

    if (!removePilha(&b->pilha, &codigo))
        return 0;

    *saida = decodificar(codigo);
    return 1;
}

void liberar_baralho(Baralho *b)
{
    free(b);
}
