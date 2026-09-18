# Truco no Bar do Pato — Relatório da 1ª Unidade

**Universidade SENAI CIMATEC · Engenharia da Computação**
**Disciplina:** Algoritmos e Estrutura de Dados
**Docente:** Márcio Renê Brandão Soussa
**Entrega:** 1ª Unidade

**Equipe:** Lucas, Rafael, Pedro, Joao e Arthur

## 1. Participação de cada componente

| Componente | Módulo | Arquivos | O que foi feito |
|---|---|---|---|
| **Lucas** | 1 · Jogadores, duplas e mesa | `include/jogador.h`, `src/jogador.c` | Structs `Jogador`, `Dupla` e `Mesa`. Cadastro do jogador com o nome truncado com segurança, ligação de mão dupla entre jogador e dupla, e montagem da mesa alternando as duplas (D1, D2, D1, D2). |
| **Rafael** | 2 · Baralho | `include/baralho.h`, `src/baralho.c` | Enums `Naipe` e `Valor` na ordem de força do truco e struct `Carta`. Baralho implementado como **pilha** com o TAD `estruturas/pilha.h`, em tipo opaco (nenhum módulo acessa carta do meio): criação das 40 cartas, embaralhamento de Fisher-Yates e retirada só pelo topo (`puxar_do_topo`). |
| **Pedro** | 3 · Vira e manilha | `include/manilha.h`, `src/manilha.c` | Revelação da vira pelo topo, cálculo cíclico da manilha (4→5→…→3→4) e a função `forca_da_carta`, com a manilha acima das cartas comuns e desempate por naipe (ouros < espadas < copas < paus). Essa função já fica pronta para a 2ª unidade. |
| **Joao** | 4 · Fila de turnos (roda de jogo) | `include/fila_turno.h`, `src/fila_turno.c` | Roda de jogo implementada como **fila circular** com o TAD `estruturas/fila.h`. Mostra de quem é a vez, manda para o fim quem já jogou, avisa quando a vez volta ao "mão" e gira o "mão" a cada nova mão (`girar_mao`), sem recriar a roda. |
| **Arthur** | 5 · Fluxo principal | `src/main.c` | Integração dos módulos: leitura dos nomes, baralho e roda criados uma vez e reaproveitados, distribuição das cartas seguindo a roda, exibição da mesa, da vira e da manilha, e uma sequência de mãos mostrando o "mão" girando. |