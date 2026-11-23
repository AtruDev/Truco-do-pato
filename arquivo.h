/*
 * arquivo.h — 3ª unidade, Pacote 1: Caderneta em disco
 * Responsável: Rafael
 *
 * Grava a caderneta em um arquivo texto e a reconstrói ao abrir o
 * programa. Formato: uma linha por freguês, campos separados por ';',
 * na ordem da struct Fregues:
 *
 *     nome;partidas;maos_ganhas;trucos_pedidos;trucos_recebidos;correu
 *
 * A gravação é em PRÉ-ORDEM (nó, esquerda, direita): ao recarregar
 * inserindo na ordem do arquivo, a árvore volta com o mesmo formato.
 * Em ordem alfabética a árvore viraria uma lista (busca O(n)).
 *
 * Gravação segura: escreve em '<caminho>.tmp', fecha, apaga o oficial
 * e renomeia o .tmp. Se o programa cair no meio, o arquivo antigo (ou
 * o .tmp completo) continua inteiro.
 *
 * VAR em disco (historico.txt): cada rodada é ACRESCENTADA no fim do
 * arquivo logo que é registrada (abre com "a", escreve o bloco, fecha).
 * Nunca reescreve, então se o programa cair perde no máximo a rodada
 * que estava sendo escrita. Uma linha por registro, a 1ª letra diz o tipo:
 *
 *     P;id;AAAA-MM-DD HH:MM;nome1;nome2;nome3;nome4   inicio (ordem da mesa)
 *     R;id;mao;rodada;dupla_vencedora;qtd_cartas;qtd_eventos
 *     C;valor;naipe;nome                              uma por carta
 *     E;dupla;tipo;nome                               uma por evento
 *     F;id;pontos_d1;pontos_d2;campea                 fim da partida
 *
 * Depende de caderneta.h (Caderneta, Fregues, inserir_fregues) e
 * historico.h (Historico, NoHist, registrar_no).
 */
#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "caderneta.h"
#include "historico.h"

#define ARQUIVO_CADERNETA "caderneta.txt"
#define ARQUIVO_HISTORICO "historico.txt"

/* Grava a caderneta inteira em 'caminho', um fregues por linha,
 * em PRE-ORDEM. Escreve primeiro em '<caminho>.tmp' e so depois
 * troca pelo arquivo oficial.
 * Retorna 1 se deu certo, 0 se nao conseguiu gravar. */
int salvar_caderneta(const Caderneta *c, const char *caminho);

/* Le 'caminho' e insere cada fregues na caderneta c (que deve estar
 * vazia), na ordem do arquivo.
 * Arquivo inexistente nao e erro: a caderneta fica vazia.
 * Retorna quantos fregueses foram carregados. */
int carregar_caderneta(Caderneta *c, const char *caminho);

/* Numero da proxima partida: o maior numero ja gravado + 1
 * (1 se o arquivo nao existe). */
int proximo_id_partida(const char *caminho);

/* Acrescenta o comeco da partida 'id': data e hora de agora e os
 * 4 nomes na ordem da mesa (D1, D2, D1, D2). Retorna 1 ou 0. */
int gravar_inicio_partida(const char *caminho, int id, const Mesa *m);

/* Acrescenta uma rodada do VAR (cartas, nomes e apostas do no).
 * Chamada logo depois de cada registrar_rodada. Retorna 1 ou 0. */
int gravar_rodada(const char *caminho, int id, const NoHist *no);

/* Acrescenta o fim da partida: placar final e dupla campea (1 ou 2). */
int gravar_fim_partida(const char *caminho, int id, int pontos_d1,
                       int pontos_d2, int campea);

/* Mostra as partidas gravadas: numero, data, duplas e placar final
 * ou "interrompida". Retorna quantas partidas tem (0 = nao mostra nada). */
int listar_partidas_salvas(const char *caminho);

/* Monta um Historico novo com todas as rodadas da partida 'id'.
 * Retorna NULL se a partida nao existe ou faltar memoria.
 * Quem chama libera com liberar_historico. */
Historico *carregar_partida(const char *caminho, int id);

#endif /* ARQUIVO_H */
