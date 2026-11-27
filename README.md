# 🦆 Truco no Bar do Pato

Jogo de **truco paulista para 4 jogadores (2 duplas)** escrito em **C99**, feito como trabalho da disciplina
**Algoritmos e Estrutura de Dados** (Engenharia da Computação · Universidade SENAI CIMATEC).

O baralho é uma **pilha**, a roda de jogo é uma **fila circular**, a caderneta de fregueses é uma
**árvore binária de busca**, o VAR é uma **lista duplamente encadeada** e o mural é ordenado com **mergesort**.

O projeto tem duas versões, que usam os mesmos módulos de regras:

| Versão | Onde está | Interface | Precisa de |
|---|---|---|---|
| **v2 · gráfica** | branch `main` (tag `v2-grafica-final`) | Janela com mouse e teclado, sons e animações ([raylib](https://www.raylib.com/)) | gcc + raylib **6.0** (Windows) |
| **v1 · terminal** | tag `v1-terminal-final` | Menu de texto no terminal | Só o gcc |

---

## Sumário

1. [Como rodar a versão gráfica (Windows)](#1-como-rodar-a-versão-gráfica-windows)
2. [Como rodar a versão de terminal](#2-como-rodar-a-versão-de-terminal)
3. [Como jogar](#3-como-jogar)
4. [Testes automáticos](#4-testes-automáticos)
5. [Problemas comuns](#5-problemas-comuns)
6. [Estrutura do projeto](#6-estrutura-do-projeto)
7. [Equipe e módulos](#7-equipe-e-módulos)
8. [Licenças](#8-licenças)

---

## 1. Como rodar a versão gráfica (Windows)

O caminho abaixo foi testado no Windows 11 com MSYS2 (gcc 15.2) e raylib 6.0. Ele leva uns 10 minutos
na primeira vez. Depois disso, recompilar é só o passo 5.

### Passo 1: instalar o MSYS2 (compilador C)

1. Baixe e instale o MSYS2 em <https://www.msys2.org/>. Mantenha a pasta padrão `C:\msys64`.
2. No menu Iniciar, abra o terminal **"MSYS2 UCRT64"** (o ícone amarelo/laranja).
   **Todos os comandos daqui em diante são digitados nesse terminal.**
3. Atualize e instale o gcc, o make e o git:

   ```sh
   pacman -Syu        # se ele pedir para fechar o terminal, feche, abra de novo e rode outra vez
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make git tar
   ```

4. Confira se deu certo:

   ```sh
   gcc --version          # deve mostrar "gcc.exe (Rev..., Built by MSYS2 project) ..."
   mingw32-make --version
   ```

> **Já usa o Git Bash?** Também funciona, desde que `C:\msys64\ucrt64\bin` esteja no `PATH` do Windows
> (para o Git Bash achar o `gcc` e o `mingw32-make`).

### Passo 2: baixar o projeto

```sh
cd ~                    # ou outra pasta de sua escolha, ex.: cd /c/Users/SEU_USUARIO/Documents
git clone https://github.com/AtruDev/Truco-do-pato.git
cd Truco-do-pato
```

### Passo 3: baixar o código-fonte da raylib 6.0

O jogo usa funções da **raylib 6.0**. O pacote da raylib que vem no `pacman` pode ser de uma versão
anterior (5.5), que **não compila** este projeto (veja [Problemas comuns](#5-problemas-comuns)). Por isso a
raylib é compilada a partir do código-fonte, com um script que já vem no repositório:

```sh
curl -L -o raylib-6.0.tar.gz https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
tar xzf raylib-6.0.tar.gz
```

Isso cria a pasta `raylib-6.0/` dentro do projeto (ela é ignorada pelo git).

### Passo 4: compilar a raylib (só na primeira vez)

```sh
sh ferramentas/compilar_raylib.sh raylib-6.0 raylib
```

Leva menos de 1 minuto e termina com `pronto: raylib/libraylib.a`.
A pasta `raylib/` fica com a biblioteca estática e os headers. Ela também é ignorada pelo git.

### Passo 5: compilar o jogo

```sh
mingw32-make RAYLIB_DIR=raylib
```

Deve compilar **sem nenhum aviso** e gerar o `bar_do_pato.exe` (~2 MB) na raiz do projeto.

### Passo 6: jogar

```sh
./bar_do_pato.exe
```

Ou dê **dois cliques** no `bar_do_pato.exe` pelo Explorador de Arquivos.

O `.exe` é estático: ele **não precisa da raylib nem do MSYS2 instalados** para rodar. Para levar o jogo
para outro computador, copie **o `.exe` junto com a pasta `assets/`** (as fontes e os sons ficam lá):

```
qualquer-pasta/
├── bar_do_pato.exe
└── assets/
```

### Recompilar depois de mexer no código

```sh
mingw32-make RAYLIB_DIR=raylib     # recompila
mingw32-make limpar                # apaga o .exe e os executáveis de teste
```

### E no Linux / macOS?

O código usa só C99 e a raylib, que é multiplataforma, mas o `Makefile` foi escrito para o Windows
(`-lopengl32 -lgdi32 -lwinmm`, `-mwindows`). **A versão gráfica não foi testada fora do Windows.**
Se quiser tentar no Linux, instale ou compile a raylib 6.0 e troque a linha de link por algo como:

```sh
gcc -std=c99 -Wall -Wextra src/*.c gui/*.c -Isrc -Igui -Iraylib -o bar_do_pato \
    -Lraylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

---

## 2. Como rodar a versão de terminal

A versão de terminal está guardada na tag `v1-terminal-final`. Ela só precisa do gcc (passo 1 acima),
sem raylib.

```sh
# dentro da pasta Truco-do-pato
git worktree add ../Truco-do-pato-v1 v1-terminal-final   # cria uma pasta separada com a v1
cd ../Truco-do-pato-v1

gcc -std=c99 -Wall -Wextra *.c -o bar_do_pato.exe
./bar_do_pato.exe
```

> Prefere não usar `worktree`? `git checkout v1-terminal-final` troca a pasta atual para a v1
> e `git checkout main` volta para a versão gráfica.

No Linux/macOS o mesmo comando deve funcionar (sem o `.exe` no nome), mas não foi testado.

Menu da versão de terminal:

```
[1] Nova partida
[2] Ver a caderneta (A-Z)
[3] Procurar fregues
[4] Mural do bar
[5] VAR de partidas antigas
[0] Fechar o bar
```

---

## 3. Como jogar

### Regras usadas (truco paulista)

- Baralho de **40 cartas** (sem 8, 9 e 10). Cada jogador recebe 3 cartas.
- A carta virada (**vira**) define a **manilha**: a carta seguinte na ordem `4 5 6 7 Q J K A 2 3`
  (depois do 3 volta ao 4). Entre manilhas, o desempate é pelo naipe: ouros < espadas < copas < paus.
- A mão é **melhor de 3 rodadas** e vale 1 ponto. Pedir **truco** sobe para 3, depois **seis** e **doze**.
  Quem corre de um pedido dá os pontos do valor anterior para a outra dupla.
- **Mão de onze:** a dupla com 11 pontos vê as próprias cartas e decide se joga (vale 3) ou corre (+1 para a adversária).
  Não pode pedir truco. **Mão de ferro** (11 × 11): vale 1 e não pode pedir truco.
- Ganha a dupla que chegar a **12 pontos**.

### Telas da versão gráfica

| Tela | O que faz |
|---|---|
| **Menu** | Nova partida, Continuar partida (se houver uma em andamento), Caderneta, Mural, VAR de partidas antigas, Fechar o bar |
| **Cadastro** | Nome dos 4 jogadores (2 por dupla). Avisa se é "Rosto novo" ou "Freguês da casa" |
| **Mesa** | A partida. Uma **cortina** esconde as cartas até o jogador da vez clicar, para ninguém ver a mão do outro |
| **Caderneta** | Busca de fregueses e lista A-Z com partidas, mãos ganhas, trucos pedidos/recebidos e quantas vezes correu |
| **Mural** | Ranking do bar (mãos ganhas) e o **Mural da vergonha** (taxa de quem correu do truco) |
| **VAR** | Revê a partida rodada por rodada, com linha do tempo clicável |

### Teclado

| Onde | Tecla | Ação |
|---|---|---|
| Qualquer tela | `F11` | Tela cheia (a janela também pode ser redimensionada) |
| Qualquer tela | `Esc` | Voltar ao menu |
| Cadastro | `Tab` / `Shift+Tab` · `Enter` | Trocar de campo · sentar à mesa |
| Mesa | `1` `2` `3` | Jogar a 1ª, 2ª ou 3ª carta |
| Mesa | `T` | Pedir truco / seis / doze |
| Mesa | `Espaço` ou `Enter` | Abrir a cortina / continuar |
| Mesa | `V` | Abrir o VAR da partida |
| Pedido de truco | `1` `2` `3` | Aceitar · correr · aumentar |
| Mão de onze | `1` `2` | Jogar · correr |
| VAR | `←` `→` · `Home` `End` | Rodada anterior/seguinte · primeira/última |

Tudo também pode ser feito com o mouse.

### Arquivos que o jogo cria

Ficam na mesma pasta do executável e não vão para o git:

| Arquivo | Conteúdo |
|---|---|
| `caderneta.txt` | Os fregueses e suas estatísticas (salvo a cada mão e ao fechar o jogo) |
| `historico.txt` | O VAR de todas as partidas, usado em "VAR de partidas antigas" |

Para "zerar o bar", basta apagar esses dois arquivos com o jogo fechado.

---

## 4. Testes automáticos

Os testes do motor da partida não precisam da raylib:

```sh
mingw32-make teste
```

Saída esperada (resumida):

```
1. 1000 partidas com acoes aleatorias
   5468 maos (377 de onze, 31 de ferro, 1876 terminadas por corrida)
...
5. Memoria
   blocos vivos no fim: 0

42/42 testes passaram.
```

O teste joga 1000 partidas com ações aleatórias, confere as sequências de truco, a mão de onze e de ferro,
compara o VAR em disco com o da memória e conta `malloc`/`free` para garantir que não sobra memória alocada.
Ele usa arquivos próprios (`teste_motor_*.txt`), então **não mexe na caderneta de verdade**.

---

## 5. Problemas comuns

| Sintoma | Causa | Solução |
|---|---|---|
| `gcc: command not found` ou `mingw32-make: command not found` | O terminal não é o **MSYS2 UCRT64**, ou faltou instalar os pacotes | Abra o "MSYS2 UCRT64" e rode o `pacman -S` do passo 1 |
| `make: command not found` | No MSYS2 o make se chama `mingw32-make` | Use `mingw32-make` |
| Erros como `too few arguments to function 'DrawCircleGradient'` | O gcc achou a raylib **5.5** do `pacman` em vez da 6.0 | Faça os passos 3 e 4 e compile com `mingw32-make RAYLIB_DIR=raylib` |
| `ld returned 5 exit status` sem outra mensagem | Bug conhecido da `libraylib.a` pronta do MSYS2 com o `ld` 2.45 | Mesma solução: compilar a raylib com o script (passo 4) |
| `Nao achei raylib-6.0/src/raylib.h` | O `tar` não rodou ou a pasta tem outro nome | Rode `ls` e passe o nome certo da pasta para o script |
| Jogo abre sem sons ou com fonte estranha | O `.exe` foi copiado sem a pasta `assets/` | Deixe `assets/` ao lado do `.exe` |
| Windows SmartScreen bloqueia o `.exe` | O executável não é assinado | "Mais informações" → "Executar assim mesmo" |

---

## 6. Estrutura do projeto

```
Truco-do-pato/
├── src/            regras do jogo (comuns às duas versões) + motor da partida
│   ├── pilha.h, fila.h     TADs genéricos de pilha e fila
│   ├── baralho.*           baralho como pilha (Fisher-Yates, compra só pelo topo)
│   ├── manilha.*           vira, manilha e força da carta
│   ├── jogador.*           jogadores, duplas e mesa
│   ├── fila_turno.*        roda de jogo (fila circular)
│   ├── rodada.*            vencedor da rodada e da mão
│   ├── truco.*             máquina de estados da aposta (1 → 3 → 6 → 12)
│   ├── historico.*         VAR (lista duplamente encadeada)
│   ├── caderneta.*         fregueses em árvore binária de busca
│   ├── arquivo.*           caderneta.txt e historico.txt em disco
│   ├── mural.*             mergesort, mural e mural da vergonha
│   └── partida.*           motor da partida (máquina de estados, sem printf)
├── gui/            telas da versão gráfica (raylib)
├── testes/         teste_motor.c e o contador de memória
├── assets/         fontes DejaVu, sons .wav e licenças
├── ferramentas/    compilar_raylib.sh
├── Makefile
└── RELATORIO.md    relatório da 1ª unidade
```

Os headers `pilha.h` e `fila.h` definem as funções dentro do próprio arquivo, então cada um é incluído em
**um único** `.c` (`baralho.c` e `fila_turno.c`). Por isso `Baralho` e `FilaTurno` são tipos opacos:
os outros módulos só usam ponteiros para eles.

O motor (`partida.c`) não desenha nem imprime nada. As telas chamam as ações do motor e só desenham o
estado dele, o que permite testar as regras sem abrir janela.

---

## 7. Equipe e módulos

| Integrante | Módulos |
|---|---|
| **Arthur** | Fluxo principal (`main.c` da v1), motor da partida (`partida.*`), testes do motor, janela principal e mesa (`main_gui.c`, `gui_mesa.c`) |
| **Lucca** | Jogadores e duplas (`jogador.*`), caderneta em árvore (`caderneta.*`), telas de menu, cadastro e caderneta |
| **Rafael** | Baralho (`baralho.*`), caderneta e VAR em disco (`arquivo.*`), telas do VAR |
| **Pedro** | Vira e manilha (`manilha.*`), rodada (`rodada.*`), mural (`mural.*`), base gráfica e tela do mural |
| **Joao** | Fila de turnos / roda de jogo (`fila_turno.*`) |

Docente: Márcio Renê Brandão Soussa · Relatório da 1ª unidade: [`RELATORIO.md`](RELATORIO.md)

---

## 8. Licenças

- **Fontes:** DejaVu Sans 2.37 (licença livre, ver `assets/fonte/LICENSE-DejaVu.txt`).
- **Sons:** gerados por script, domínio público (CC0).
- **Cartas e logo:** desenhados pelo próprio jogo. Opcionalmente, o jogo usa as imagens do
  [Kenney Playing Cards Pack](https://kenney.nl/assets/playing-cards-pack) se elas forem colocadas em `assets/cartas/`.
- **raylib 6.0:** licença zlib.

Detalhes em [`assets/LICENCAS.txt`](assets/LICENCAS.txt).
