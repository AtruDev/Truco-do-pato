# Bar do Pato

Jogo de cartas estilo truco em C — trabalho de AED.

## Estrutura

```
estruturas/ TADs pilha.h e fila.h
include/   headers (.h) 
src/       implementações (.c) e main.c
Makefile
```

## Módulos e responsáveis

| # | Módulo | Arquivos | Responsável | Conteúdo |
|---|--------|----------|-------------|----------|
| 1 | Jogadores | `jogador.h` / `jogador.c` | Lucca | `Jogador`, `Dupla`, `Mesa`; `cadastrar_jogador`, `criar_dupla`, `montar_mesa` |
| 2 | Baralho | `baralho.h` / `baralho.c` | Rafael | `Carta`, `Baralho` (pilha, opaco); `criar_baralho`, `embaralhar` (Fisher-Yates), `puxar_do_topo`, `liberar_baralho` |
| 3 | Manilha | `manilha.h` / `manilha.c` | Pedro | `revelar_vira`, `calcular_manilha`, `forca_da_carta` |
| 4 | Fila de turnos | `fila_turno.h` / `fila_turno.c` | Joao | `FilaTurno` (fila, opaca); `criar_fila`, `proximo_jogador`, `jogador_passou`, `girar_mao`, `liberar_fila` |
| 5 | Fluxo principal | `main.c` | Arthur | Integração, roda de jogo girando a cada mão, distribuição pelo topo e exibição na tela |

Dependências entre headers: `baralho.h` ← `jogador.h` ← `fila_turno.h`, e `baralho.h` ← `manilha.h`.

`pilha.h` e `fila.h` definem as funções dentro do próprio header, então cada um só pode ser incluído em **um** `.c`: `pilha.h` só em `baralho.c` e `fila.h` só em `fila_turno.c`. Por isso `Baralho` e `FilaTurno` são tipos opacos (os outros módulos usam só ponteiro).

## Regras para trabalhar em paralelo

- Cada pessoa edita **só o próprio `.c`** (e `main.c` no caso do Arthur).
- O projeto compila sem avisos com `-Wall -Wextra` — mantenha assim a cada commit.

Relatório da 1ª unidade: [`RELATORIO.md`](RELATORIO.md).

## Compilar

```sh
make          # Linux/macOS
mingw32-make  # Windows (MinGW/MSYS2)
make run
make clean
```
