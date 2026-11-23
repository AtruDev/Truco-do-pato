/*
 * teste_motor.c — 3ª unidade, Pacote 6: testes do motor da partida
 * Responsável: Arthur
 *
 * Como o motor não lê nem escreve nada na tela, dá para testá-lo
 * jogando sozinho. Usa arquivos próprios (teste_motor_*.txt, via -D no
 * Makefile), que são apagados no fim: os do bar nunca são tocados.
 *
 * Compilar e rodar pela pasta da versão 2:
 *     make teste
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partida.h"
#include "arquivo.h"
#include "rodada.h"

#define PARTIDAS_ALEATORIAS 1000
#define MAX_ACOES           100000  /* por partida: passou disso, travou */

/* ------------------------------------------------------------------ */
/* Placar dos testes                                                   */
/* ------------------------------------------------------------------ */

typedef struct {
    int ok, falhas;
} Placar;

static void conferir(Placar *pl, int condicao, const char *descricao)
{
    if (condicao) {
        pl->ok++;
    } else {
        pl->falhas++;
        printf("  FALHOU: %s\n", descricao);
    }
}

static void apagar_arquivos(void)
{
    remove(ARQUIVO_CADERNETA);
    remove(ARQUIVO_CADERNETA ".tmp");
    remove(ARQUIVO_HISTORICO);
}

static const char *NOMES[NUM_JOGADORES] = { "Ana", "Bia", "Caio", "Davi" };

/* ------------------------------------------------------------------ */
/* Invariantes                                                         */
/* ------------------------------------------------------------------ */

/* Cartas nas mãos + na mesa = 12 menos as 4 de cada rodada já fechada. */
static int cartas_conferem(const Partida *p)
{
    int i, total = p->rodada.qtd;

    if (p->estado == EST_FIM_MAO || p->estado == EST_FIM_PARTIDA)
        return 1; /* mão fechada (pode ter acabado no meio, por corrida) */
    for (i = 0; i < NUM_JOGADORES; i++)
        total += p->jogadores[i].qtd_cartas;
    return total == NUM_JOGADORES * CARTAS_NA_MAO -
                    NUM_JOGADORES * (p->num_rodada - 1);
}

static int valor_valido(const Partida *p)
{
    int v = p->aposta.valor;
    return v == 1 || v == 3 || v == 6 || v == 12;
}

/* Tenta uma ação que NÃO vale no estado atual e confere que ela
 * retorna 0 sem mudar um byte da partida. Retorna 1 se tudo certo. */
static int acao_errada_nao_muda(Partida *p, int sorteio)
{
    static Partida antes; /* grande demais para a pilha a cada chamada */
    int r = 0, tentou = 1;

    memcpy(&antes, p, sizeof *p);
    switch (p->estado) {
    case EST_VEZ_DE_JOGAR:
        switch (sorteio % 5) {
        case 0: r = acao_responder(p, ACEITAR); break;
        case 1: r = acao_continuar(p); break;
        case 2: r = acao_decidir_onze(p, 1); break;
        case 3: r = acao_jogar_carta(p, -1); break;
        default: r = acao_jogar_carta(p, jogador_da_vez(p)->qtd_cartas); break;
        }
        break;
    case EST_RESPONDER_PEDIDO:
        switch (sorteio % 4) {
        case 0: r = acao_jogar_carta(p, 0); break;
        case 1: r = acao_pedir(p); break;
        case 2: r = acao_continuar(p); break;
        default:
            if (p->aposta.nivel >= NIVEL_MAXIMO)
                r = acao_responder(p, AUMENTAR); /* não sobe do doze */
            else
                r = acao_decidir_onze(p, 0);
            break;
        }
        break;
    case EST_DECIDIR_ONZE:
        switch (sorteio % 3) {
        case 0: r = acao_jogar_carta(p, 0); break;
        case 1: r = acao_pedir(p); break;
        default: r = acao_continuar(p); break;
        }
        break;
    case EST_FIM_RODADA:
    case EST_FIM_MAO:
        switch (sorteio % 3) {
        case 0: r = acao_jogar_carta(p, 0); break;
        case 1: r = acao_pedir(p); break;
        default: r = acao_responder(p, CORRER); break;
        }
        break;
    case EST_FIM_PARTIDA:
        r = acao_continuar(p) + acao_jogar_carta(p, 0) + acao_pedir(p);
        break;
    default:
        tentou = 0;
    }
    return !tentou || (r == 0 && memcmp(&antes, p, sizeof *p) == 0);
}

/* Uma ação válida sorteada para o estado atual. Retorna o que ela
 * retornou (tem que ser 1). */
static int acao_aleatoria(Partida *p)
{
    Jogador *j;
    int s = rand() % 100;

    switch (p->estado) {
    case EST_DECIDIR_ONZE:
        return acao_decidir_onze(p, s < 60);
    case EST_VEZ_DE_JOGAR:
        if (pode_pedir_agora(p) && s < 15)
            return acao_pedir(p);
        j = jogador_da_vez(p);
        return acao_jogar_carta(p, rand() % j->qtd_cartas);
    case EST_RESPONDER_PEDIDO:
        if (s < 50)
            return acao_responder(p, ACEITAR);
        if (s < 75 || p->aposta.nivel >= NIVEL_MAXIMO)
            return acao_responder(p, CORRER);
        return acao_responder(p, AUMENTAR);
    case EST_FIM_RODADA:
    case EST_FIM_MAO:
        return acao_continuar(p);
    default:
        return 0;
    }
}

/* O VAR gravado no disco tem que ser igual, nó a nó, ao da memória. */
static int var_do_disco_igual(const Partida *p)
{
    Historico *h = carregar_partida(ARQUIVO_HISTORICO, p->id_partida);
    const NoHist *a, *b;
    int i, igual = 1;

    if (h == NULL)
        return tamanho_historico(p->historico) == 0;
    if (tamanho_historico(h) != tamanho_historico(p->historico))
        igual = 0;
    for (a = primeira_rodada(p->historico), b = primeira_rodada(h);
         igual && a != NULL && b != NULL; a = a->prox, b = b->prox) {
        if (a->num_mao != b->num_mao || a->num_rodada != b->num_rodada ||
            a->rodada.qtd != b->rodada.qtd ||
            a->rodada.dupla_vencedora != b->rodada.dupla_vencedora ||
            a->qtd_eventos != b->qtd_eventos)
            igual = 0;
        for (i = 0; igual && i < a->rodada.qtd; i++)
            if (a->rodada.cartas[i].valor != b->rodada.cartas[i].valor ||
                a->rodada.cartas[i].naipe != b->rodada.cartas[i].naipe ||
                strcmp(a->nomes[i], b->nomes[i]) != 0)
                igual = 0;
        for (i = 0; igual && i < a->qtd_eventos; i++)
            if (a->eventos[i].tipo != b->eventos[i].tipo ||
                a->eventos[i].dupla != b->eventos[i].dupla ||
                strcmp(a->eventos[i].nome, b->eventos[i].nome) != 0)
                igual = 0;
    }
    liberar_historico(h);
    return igual;
}

/* ------------------------------------------------------------------ */
/* 1. Partidas aleatórias                                              */
/* ------------------------------------------------------------------ */

static void teste_partidas_aleatorias(Placar *pl)
{
    static Partida p;
    Caderneta c;
    int k, acoes, terminaram = 0, cartas_ok = 1, valor_ok = 1;
    int errada_ok = 1, aceitas_ok = 1, var_ok = 1, maos = 0, onzes = 0,
        ferros = 0, corridas = 0;
    long maos_ganhas_motor = 0, maos_ganhas_ficha = 0;

    printf("1. %d partidas com acoes aleatorias\n", PARTIDAS_ALEATORIAS);
    iniciar_caderneta(&c);

    for (k = 0; k < PARTIDAS_ALEATORIAS; k++) {
        int ultima_mao = 0;

        if (!iniciar_partida(&p, NOMES, &c)) {
            conferir(pl, 0, "iniciar_partida");
            break;
        }
        for (acoes = 0; p.estado != EST_FIM_PARTIDA && acoes < MAX_ACOES;
             acoes++) {
            if (p.num_mao != ultima_mao) {
                ultima_mao = p.num_mao;
                maos++;
                if (p.tipo == MAO_DE_ONZE) onzes++;
                if (p.tipo == MAO_DE_FERRO) ferros++;
            }
            if (!acao_errada_nao_muda(&p, rand()))
                errada_ok = 0;
            if (acao_aleatoria(&p) != 1)
                aceitas_ok = 0;
            if (p.estado == EST_FIM_MAO && p.correram && p.dono_da_mao != -1)
                corridas++;
            if (p.estado == EST_FIM_MAO && p.dono_da_mao != -1)
                maos_ganhas_motor += 2;
            if (!cartas_conferem(&p))
                cartas_ok = 0;
            if (!valor_valido(&p))
                valor_ok = 0;
        }
        if (p.estado == EST_FIM_PARTIDA &&
            (p.duplas[0].pontos >= PONTOS_PARA_VENCER) !=
            (p.duplas[1].pontos >= PONTOS_PARA_VENCER) &&
            p.duplas[p.campea].pontos >= PONTOS_PARA_VENCER)
            terminaram++;
        if (!acao_errada_nao_muda(&p, rand()))
            errada_ok = 0;
        if (k % 50 == 0 && !var_do_disco_igual(&p))
            var_ok = 0;
        encerrar_partida(&p);
    }

    for (k = 0; k < NUM_JOGADORES; k++)
        maos_ganhas_ficha += buscar_fregues(&c, NOMES[k])->maos_ganhas;

    printf("   %d maos (%d de onze, %d de ferro, %d terminadas por corrida)\n",
           maos, onzes, ferros, corridas);
    conferir(pl, terminaram == PARTIDAS_ALEATORIAS,
             "todas terminam com UMA dupla >= 12, sem travar");
    conferir(pl, aceitas_ok, "toda acao valida sorteada foi aceita");
    conferir(pl, cartas_ok, "cartas nas maos + mesa conferem com a rodada");
    conferir(pl, valor_ok, "valor da mao sempre 1, 3, 6 ou 12");
    conferir(pl, errada_ok, "acao no estado errado retorna 0 e nao muda nada");
    conferir(pl, var_ok, "VAR do historico.txt igual ao da memoria (1 a cada 50)");
    conferir(pl, onzes > 0 && ferros > 0, "houve maos de onze e de ferro");
    conferir(pl, buscar_fregues(&c, "ana")->partidas == PARTIDAS_ALEATORIAS,
             "cada um sentou nas 1000 partidas (busca 'ana' acha 'Ana')");
    conferir(pl, maos_ganhas_motor == maos_ganhas_ficha,
             "maos_ganhas das fichas = 2 por mao com dono");
    liberar_caderneta(&c);
    apagar_arquivos();
}

/* ------------------------------------------------------------------ */
/* 2. Sequências fixas da 2ª entrega                                   */
/* ------------------------------------------------------------------ */

/* Começa uma partida nova até cair numa mão normal com a vez em
 * EST_VEZ_DE_JOGAR (a 1ª mão sempre é normal). */
static int nova(Partida *p, Caderneta *c)
{
    apagar_arquivos();
    iniciar_caderneta(c);
    return iniciar_partida(p, NOMES, c) && p->estado == EST_VEZ_DE_JOGAR &&
           p->tipo == MAO_NORMAL;
}

/* Joga sempre a 1ª carta e continua até a mão fechar. */
static void jogar_ate_fim_da_mao(Partida *p)
{
    int limite = 100;

    while (p->estado != EST_FIM_MAO && limite-- > 0) {
        if (p->estado == EST_VEZ_DE_JOGAR)
            acao_jogar_carta(p, 0);
        else if (p->estado == EST_FIM_RODADA)
            acao_continuar(p);
        else
            break;
    }
}

static void fim(Partida *p, Caderneta *c)
{
    encerrar_partida(p);
    liberar_caderneta(c);
}

static void teste_sequencias(Placar *pl)
{
    static Partida p;
    Caderneta c;
    int da, db;
    Jogador *a, *b;

    printf("2. Sequencias fixas de truco\n");

    /* pede -> aceita: a mão vale 3 */
    if (nova(&p, &c)) {
        a = jogador_da_vez(&p);
        da = dupla_do_jogador(&p, a);
        conferir(pl, acao_pedir(&p) && p.estado == EST_RESPONDER_PEDIDO,
                 "pedir truco leva a EST_RESPONDER_PEDIDO");
        b = jogador_da_vez(&p);
        conferir(pl, dupla_do_jogador(&p, b) == 1 - da,
                 "quem responde e da outra dupla");
        conferir(pl, acao_responder(&p, ACEITAR) && jogador_da_vez(&p) == a,
                 "aceitou: o mesmo jogador continua na vez");
        conferir(pl, !pode_pedir_agora(&p), "quem pediu nao pede de novo");
        jogar_ate_fim_da_mao(&p);
        conferir(pl, p.pontos_da_mao == (p.dono_da_mao == -1 ? 0 : 3),
                 "pede -> aceita: a mao vale 3");
        conferir(pl, a->ficha->trucos_pedidos == 1 &&
                     b->ficha->trucos_recebidos == 1 && b->ficha->correu == 0,
                 "contadores: 1 pedido, 1 recebido");
        fim(&p, &c);
    }

    /* pede -> corre: quem pediu leva 1 */
    if (nova(&p, &c)) {
        a = jogador_da_vez(&p);
        da = dupla_do_jogador(&p, a);
        acao_pedir(&p);
        b = jogador_da_vez(&p);
        acao_responder(&p, CORRER);
        conferir(pl, p.estado == EST_FIM_MAO && p.dono_da_mao == da &&
                     p.pontos_da_mao == 1 && p.duplas[da].pontos == 1,
                 "pede -> corre: quem pediu leva 1");
        conferir(pl, b->ficha->correu == 1 && b->ficha->trucos_recebidos == 1,
                 "quem correu: correu 1 e recebeu 1");
        conferir(pl, tamanho_historico(p.historico) == 1,
                 "a rodada parcial foi para o VAR");
        fim(&p, &c);
    }

    /* pede -> aumenta -> corre: quem aumentou leva 3 */
    if (nova(&p, &c)) {
        a = jogador_da_vez(&p);
        da = dupla_do_jogador(&p, a);
        acao_pedir(&p);
        b = jogador_da_vez(&p);
        db = dupla_do_jogador(&p, b);
        conferir(pl, acao_responder(&p, AUMENTAR) && jogador_da_vez(&p) == a,
                 "aumentou: agora quem responde e quem pediu");
        acao_responder(&p, CORRER);
        conferir(pl, p.dono_da_mao == db && p.pontos_da_mao == 3 &&
                     p.duplas[db].pontos == 3,
                 "pede -> aumenta -> corre: quem aumentou leva 3");
        conferir(pl, a->ficha->trucos_pedidos == 1 &&
                     a->ficha->trucos_recebidos == 1 && a->ficha->correu == 1 &&
                     b->ficha->trucos_pedidos == 1 &&
                     b->ficha->trucos_recebidos == 1 && b->ficha->correu == 0,
                 "aumentar conta como recebido E como pedido");
        fim(&p, &c);
    }

    /* pede -> aumenta -> aceita: a mão vale 6 */
    if (nova(&p, &c)) {
        acao_pedir(&p);
        acao_responder(&p, AUMENTAR);
        acao_responder(&p, ACEITAR);
        conferir(pl, p.aposta.valor == 6 && p.estado == EST_VEZ_DE_JOGAR,
                 "pede -> aumenta -> aceita: vale 6");
        jogar_ate_fim_da_mao(&p);
        conferir(pl, p.pontos_da_mao == (p.dono_da_mao == -1 ? 0 : 6),
                 "a mao de 6 da 6 pontos");
        fim(&p, &c);
    }

    /* truco -> seis -> doze -> aceita: vale 12 e fecha a partida */
    if (nova(&p, &c)) {
        acao_pedir(&p);
        acao_responder(&p, AUMENTAR);
        acao_responder(&p, AUMENTAR);
        conferir(pl, p.aposta.nivel == NIVEL_MAXIMO &&
                     acao_responder(&p, AUMENTAR) == 0,
                 "no doze nao da para aumentar");
        acao_responder(&p, ACEITAR);
        conferir(pl, p.aposta.valor == 12 && !pode_pedir_agora(&p),
                 "... -> doze -> aceita: vale 12, ninguem pede mais");
        jogar_ate_fim_da_mao(&p);
        if (p.dono_da_mao != -1) {
            conferir(pl, p.pontos_da_mao == 12 && acao_continuar(&p) &&
                         p.estado == EST_FIM_PARTIDA &&
                         p.campea == p.dono_da_mao,
                     "mao de 12 encerra a partida");
        }
        fim(&p, &c);
    }
}

/* ------------------------------------------------------------------ */
/* 3. Mão de onze e mão de ferro                                       */
/* ------------------------------------------------------------------ */

/* Fecha a 1ª mão (sem truco) e põe o placar em p0 x p1 antes da próxima. */
static int preparar_placar(Partida *p, Caderneta *c, int p0, int p1)
{
    if (!nova(p, c))
        return 0;
    jogar_ate_fim_da_mao(p);
    p->duplas[0].pontos = p0;
    p->duplas[1].pontos = p1;
    return acao_continuar(p);
}

static void teste_mao_de_onze(Placar *pl)
{
    static Partida p;
    Caderneta c;
    int dono, limite;

    printf("3. Mao de onze e mao de ferro\n");

    if (preparar_placar(&p, &c, 11, 5)) {
        conferir(pl, p.estado == EST_DECIDIR_ONZE && p.tipo == MAO_DE_ONZE &&
                     p.dupla_onze == 0 &&
                     dupla_do_jogador(&p, jogador_da_vez(&p)) == 0,
                 "11 x 5: a dupla 1 decide a mao de onze");
        acao_decidir_onze(&p, 0);
        conferir(pl, p.estado == EST_FIM_MAO && p.duplas[1].pontos == 6 &&
                     p.duplas[0].pontos == 11,
                 "correu da mao de onze: adversaria +1");
        conferir(pl, p.jogadores[0].ficha->correu == 0,
                 "correr da mao de onze nao conta como correu do truco");
        acao_continuar(&p);
        conferir(pl, p.estado == EST_DECIDIR_ONZE,
                 "a proxima mao continua sendo de onze");
        fim(&p, &c);
    }

    if (preparar_placar(&p, &c, 4, 11)) {
        acao_decidir_onze(&p, 1);
        conferir(pl, p.aposta.valor == 3 && p.aposta.bloqueada &&
                     !pode_pedir_agora(&p) && acao_pedir(&p) == 0,
                 "jogou a mao de onze: vale 3 e ninguem pede truco");
        limite = 100;
        while (p.estado != EST_FIM_MAO && limite-- > 0) {
            if (pode_pedir_agora(&p))
                conferir(pl, 0, "truco liberado na mao de onze");
            if (p.estado == EST_VEZ_DE_JOGAR)
                acao_jogar_carta(&p, 0);
            else
                acao_continuar(&p);
        }
        dono = p.dono_da_mao;
        if (dono == 1)
            conferir(pl, p.duplas[1].pontos == 14 && (acao_continuar(&p),
                     p.estado == EST_FIM_PARTIDA),
                     "jogou e ganhou a de onze: vai a 14 e vence");
        else if (dono == 0)
            conferir(pl, p.duplas[0].pontos == 7,
                     "jogou e perdeu a de onze: adversaria +3");
        fim(&p, &c);
    }

    if (preparar_placar(&p, &c, 11, 11)) {
        conferir(pl, p.estado == EST_VEZ_DE_JOGAR && p.tipo == MAO_DE_FERRO &&
                     p.aposta.valor == 1 && !pode_pedir_agora(&p),
                 "11 x 11: mao de ferro, vale 1 e sem truco");
        jogar_ate_fim_da_mao(&p);
        if (p.dono_da_mao != -1)
            conferir(pl, acao_continuar(&p) && p.estado == EST_FIM_PARTIDA,
                     "quem leva a mao de ferro fecha a partida");
        fim(&p, &c);
    }

    if (preparar_placar(&p, &c, 10, 3)) {
        conferir(pl, p.tipo == MAO_NORMAL && pode_pedir_agora(&p),
                 "com 10 a mao e normal e o truco e liberado");
        fim(&p, &c);
    }
}

/* ------------------------------------------------------------------ */
/* 4. Nomes                                                            */
/* ------------------------------------------------------------------ */

static void teste_nomes(Placar *pl)
{
    const char *aceitos[2] = { "Lucca", "Bia" };

    printf("4. problema_no_nome\n");
    conferir(pl, problema_no_nome("", aceitos, 0) != NULL, "vazio recusado");
    conferir(pl, problema_no_nome("   ", aceitos, 0) != NULL, "so espacos recusado");
    conferir(pl, problema_no_nome("a;b", aceitos, 0) != NULL, "';' recusado");
    conferir(pl, problema_no_nome("lucca", aceitos, 2) != NULL,
             "repetido na mesa (sem diferenciar maiusculas) recusado");
    conferir(pl, problema_no_nome("Caio", aceitos, 2) == NULL, "nome bom aceito");
    conferir(pl, problema_no_nome("Lucca", aceitos, 0) == NULL,
             "so compara com os n ja aceitos");
}

int main(void)
{
    Placar pl = { 0, 0 };

    srand(2026);
    apagar_arquivos();

    teste_nomes(&pl);
    teste_sequencias(&pl);
    teste_mao_de_onze(&pl);
    teste_partidas_aleatorias(&pl);

    printf("5. Memoria\n");
    conferir(&pl, blocos_vivos() == 0, "nenhum bloco vivo no fim");
    printf("   blocos vivos no fim: %ld\n", blocos_vivos());

    apagar_arquivos();
    printf("\n%d/%d testes passaram.\n", pl.ok, pl.ok + pl.falhas);
    return pl.falhas == 0 ? 0 : 1;
}

/* ------------------------------------------------------------------ */
/* Contador de memória (ver conta_memoria.h)                           */
/* ------------------------------------------------------------------ */
#undef malloc
#undef calloc
#undef realloc
#undef free

static long *vivos(void)
{
    static long n = 0;
    return &n;
}

void *conta_malloc(size_t n)
{
    void *p = malloc(n);
    if (p != NULL)
        (*vivos())++;
    return p;
}

void *conta_calloc(size_t q, size_t n)
{
    void *p = calloc(q, n);
    if (p != NULL)
        (*vivos())++;
    return p;
}

void *conta_realloc(void *p, size_t n)
{
    void *novo = realloc(p, n);
    if (p == NULL && novo != NULL)
        (*vivos())++;
    return novo;
}

void conta_free(void *p)
{
    if (p != NULL)
        (*vivos())--;
    free(p);
}

long blocos_vivos(void)
{
    return *vivos();
}
