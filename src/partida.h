/*
 * partida.h — 3ª unidade, Pacote 6 (versão 2): Motor da partida
 * Responsável: Arthur
 *
 * O jogar_mao + resolver_pedido + laço da partida da versão 1,
 * reescritos como MÁQUINA DE ESTADOS. O motor nunca espera entrada e
 * nunca imprime nada: guarda em que ponto a partida está, recebe uma
 * ação ("jogar a 2ª carta", "pedir truco") e muda de estado. Assim a
 * mesma partida roda numa janela que redesenha 60 vezes por segundo.
 *
 *   EST_DECIDIR_ONZE      a dupla com 11 joga (vale 3) ou corre (+1 deles)
 *   EST_VEZ_DE_JOGAR      o jogador da vez joga uma carta ou pede truco
 *   EST_RESPONDER_PEDIDO  o adversário aceita, corre ou aumenta
 *   EST_FIM_RODADA        pausa para a tela mostrar quem levou a rodada
 *   EST_FIM_MAO           pausa para a tela mostrar os pontos da mão
 *   EST_FIM_PARTIDA       alguém chegou a 12
 *
 * Tudo que os pacotes 4 e 5 fazem no main.c da versão 1 (contadores
 * da caderneta, VAR em disco, salvar a cada mão, mão de onze e de
 * ferro) acontece aqui, nos mesmos pontos e com as mesmas regras.
 *
 * Depende de caderneta.h, mural.h, fila_turno.h, truco.h, historico.h
 * (e, no .c, de baralho, manilha, rodada e arquivo).
 */
#ifndef PARTIDA_H
#define PARTIDA_H

#include "caderneta.h"
#include "mural.h"
#include "fila_turno.h"
#include "truco.h"
#include "historico.h"

#define PONTOS_PARA_VENCER 12
#define PONTOS_MAO_DE_ONZE 11
#define RODADAS_POR_MAO    3

typedef enum { MAO_NORMAL, MAO_DE_ONZE, MAO_DE_FERRO } TipoMao;

typedef enum {
    EST_DECIDIR_ONZE, EST_VEZ_DE_JOGAR, EST_RESPONDER_PEDIDO,
    EST_FIM_RODADA, EST_FIM_MAO, EST_FIM_PARTIDA
} EstadoPartida;

/* Tudo da partida. As interfaces LEEM os campos para desenhar,
 * mas so mudam alguma coisa pelas funcoes acao_*.
 * A struct guarda ponteiros para dentro dela mesma (mesa -> jogadores),
 * entao uma Partida nunca deve ser copiada depois de iniciar_partida. */
typedef struct {
    Jogador jogadores[NUM_JOGADORES];
    Dupla duplas[2];
    Mesa mesa;
    Baralho *baralho;
    FilaTurno *fila;
    Historico *historico;      /* VAR desta partida */
    Caderneta *caderneta;      /* do bar: a partida so usa */
    int id_partida;            /* numero no historico.txt */

    EstadoPartida estado;
    int num_mao, num_rodada;
    TipoMao tipo;
    Carta vira;
    Valor manilha;
    Aposta aposta;
    Rodada rodada;             /* cartas na mesa agora */
    int resultados[3];         /* vencedor de cada rodada desta mao */
    EventoAposta eventos[MAX_EVENTOS];
    int qtd_eventos;
    Jogador *abre;             /* quem abre a rodada atual */
    Jogador *responde;         /* quem responde o pedido pendente */

    int dono_da_mao;           /* em EST_FIM_MAO: 0, 1 ou -1 (empatou) */
    int pontos_da_mao;         /* em EST_FIM_MAO: quanto valeu */
    MaoMaisCara mais_cara;
    char aviso[96];            /* ultima coisa que aconteceu, para a tela */

    /* --- campos a mais da versao 2 (combinados no grupo) --- */
    Jogador *pediu;            /* quem fez o pedido pendente */
    int dupla_onze;            /* em EST_DECIDIR_ONZE: a dupla com 11 */
    int situacao_mao;          /* em EST_FIM_RODADA: o apurar_mao (0, 1,
                                  MAO_EMPATADA ou MAO_EM_ABERTO) */
    int idx_vencedora;         /* em EST_FIM_RODADA: indice da carta que
                                  levou em rodada.cartas, -1 se empatou */
    int correram;              /* 1 se a mao acabou porque alguem correu */
    int campea;                /* em EST_FIM_PARTIDA: 0 ou 1 */
    int falhou_caderneta;      /* 1 se o ultimo salvar_caderneta falhou */
    int falhou_var;            /* 1 se alguma gravacao do VAR falhou
                                  (a tela avisa uma vez so) */
} Partida;

/* NULL se o nome serve; senao a mensagem do problema (vazio, com ';',
 * comprido demais ou repetido entre os n nomes ja aceitos, comparando
 * com comparar_nomes). As duas interfaces usam. */
const char *problema_no_nome(const char *nome, const char *aceitos[], int n);

/* Senta os 4 (nomes[0] e [1] = dupla 1), liga cada um a sua ficha,
 * conta a partida, salva, grava o inicio no historico.txt e comeca
 * a 1a mao. Retorna 1, ou 0 se faltar memoria (nada fica alocado). */
int iniciar_partida(Partida *p, const char *nomes[NUM_JOGADORES],
                    Caderneta *c);

/* Quem precisa agir agora (NULL nos estados de fim). Na mao de onze,
 * o 1o jogador da dupla com 11 (a decisao e da dupla). */
Jogador *jogador_da_vez(const Partida *p);

/* Indice (0 ou 1) da dupla do jogador. */
int dupla_do_jogador(const Partida *p, const Jogador *j);

/* 1 se o jogador da vez pode pedir o proximo degrau agora. */
int pode_pedir_agora(const Partida *p);

/* Acoes. Retornam 1 se a acao foi aceita, 0 se nao vale no estado
 * atual (estado errado, indice invalido, pedido proibido): nada muda.
 *   acao_jogar_carta: idx = 0..qtd_cartas-1 do jogador da vez;
 *   acao_responder:   ACEITAR, CORRER ou AUMENTAR (nao vale no doze);
 *   acao_decidir_onze: jogar = 1 joga valendo 3, 0 corre;
 *   acao_continuar:   sai de EST_FIM_RODADA ou EST_FIM_MAO. */
int acao_jogar_carta(Partida *p, int idx);
int acao_pedir(Partida *p);
int acao_responder(Partida *p, Resposta r);
int acao_decidir_onze(Partida *p, int jogar);
int acao_continuar(Partida *p);

/* Libera baralho, roda e VAR. A caderneta continua (e do bar). */
void encerrar_partida(Partida *p);

#endif /* PARTIDA_H */
