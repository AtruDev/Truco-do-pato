/*
 * conta_memoria.h — só para os testes (não vai no jogo)
 *
 * Incluído em TODOS os .c do teste com -include testes/conta_memoria.h:
 * troca malloc/calloc/realloc/free por versões que contam os blocos
 * vivos, sem mexer no código dos módulos. As versões ficam no fim do
 * teste_motor.c. Substitui o valgrind/Dr. Memory quando eles não estão
 * instalados (no Linux, rodar o valgrind também).
 */
#ifndef CONTA_MEMORIA_H
#define CONTA_MEMORIA_H

#include <stdlib.h>   /* antes dos #define, para não estragar os protótipos */

void *conta_malloc(size_t n);
void *conta_calloc(size_t q, size_t n);
void *conta_realloc(void *p, size_t n);
void conta_free(void *p);
long blocos_vivos(void);

#define malloc(n)     conta_malloc(n)
#define calloc(q, n)  conta_calloc(q, n)
#define realloc(p, n) conta_realloc(p, n)
#define free(p)       conta_free(p)

#endif
