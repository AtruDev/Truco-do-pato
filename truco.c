/*
 * truco.c — Pacote 3: Truco (máquina de estados da aposta)
 *
 * VALOR_DO_NIVEL[n] é quanto a mão vale quando o degrau n é aceito.
 * Correr do degrau n entrega VALOR_DO_NIVEL[n - 1] para quem pediu,
 * ou seja, o valor que a mão tinha antes do pedido.
 *
 * 3ª unidade: iniciar_aposta_bloqueada (mão de onze / mão de ferro).
 */
#include "truco.h"

static const int VALOR_DO_NIVEL[NIVEL_MAXIMO + 1] = { 1, 3, 6, 12 };

static const char *const NOME_NIVEL[NIVEL_MAXIMO + 1] = {
    "nada", "truco", "seis", "doze"
};

void iniciar_aposta(Aposta *a)
{
    a->nivel = 0;
    a->valor = VALOR_DO_NIVEL[0];
    a->dupla_pediu = -1;
    a->aguardando = 0;
    a->bloqueada = 0;
}

void iniciar_aposta_bloqueada(Aposta *a, int valor)
{
    iniciar_aposta(a);
    a->valor = valor;
    a->bloqueada = 1;
}

int pode_pedir(const Aposta *a, int dupla)
{
    if (a->bloqueada)
        return 0; /* mão de onze ou de ferro: ninguém pede */
    if (dupla != 0 && dupla != 1)
        return 0;
    if (a->aguardando || a->nivel >= NIVEL_MAXIMO)
        return 0;
    /* depois de um aceite, a mesma dupla não pede de novo em seguida */
    return dupla != a->dupla_pediu;
}

void pedir(Aposta *a, int dupla)
{
    if (!pode_pedir(a, dupla))
        return;

    a->nivel++;
    a->dupla_pediu = dupla;
    a->aguardando = 1;
    /* a->valor só muda quando o pedido for aceito */
}

int responder(Aposta *a, Resposta r)
{
    int pontos;

    if (!a->aguardando)
        return RESPOSTA_INVALIDA;

    switch (r) {
    case ACEITAR:
        a->valor = VALOR_DO_NIVEL[a->nivel];
        a->aguardando = 0;
        return 0;

    case CORRER:
        pontos = VALOR_DO_NIVEL[a->nivel - 1];
        a->aguardando = 0;
        return pontos;

    case AUMENTAR:
        if (a->nivel >= NIVEL_MAXIMO)
            return RESPOSTA_INVALIDA; /* no doze não tem para onde subir */
        /* aceita o degrau atual e pede o próximo: a vez de responder
         * volta para a dupla que tinha pedido */
        a->valor = VALOR_DO_NIVEL[a->nivel];
        a->nivel++;
        a->dupla_pediu = 1 - a->dupla_pediu;
        return 0;
    }
    return RESPOSTA_INVALIDA;
}

int dupla_que_responde(const Aposta *a)
{
    return a->aguardando ? 1 - a->dupla_pediu : -1;
}

const char *nome_nivel(int nivel)
{
    return (nivel >= 0 && nivel <= NIVEL_MAXIMO) ? NOME_NIVEL[nivel] : "?";
}
