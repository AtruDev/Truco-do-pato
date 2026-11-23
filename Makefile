# Truco no Bar do Pato — versão 2 (gráfica, raylib)
#
#   make            compila bar_do_pato.exe (estático: roda sem instalar a raylib)
#   make teste      compila e roda os testes do motor (testes/teste_motor.c)
#   make limpar     apaga o que foi compilado
#
# A raylib vem do MSYS2 (pacman -S mingw-w64-ucrt-x86_64-raylib) ou do
# instalador da raylib para Windows. Se ela estiver numa pasta própria
# (raylib.h + libraylib.a), passe a pasta:  make RAYLIB_DIR=C:/raylib/src
#
# No MSYS2 o make se chama mingw32-make.

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra
SRC     = $(wildcard src/*.c)
GUI     = $(wildcard gui/*.c)
EXE     = bar_do_pato.exe

ifneq ($(RAYLIB_DIR),)
RAYLIB_INC = -I$(RAYLIB_DIR)
RAYLIB_LIB = -L$(RAYLIB_DIR)
endif

LIBS    = -lraylib -lopengl32 -lgdi32 -lwinmm
# -static: leva a raylib e o runtime do gcc dentro do .exe
# -mwindows: abre só a janela, sem o console preto atrás
LDFLAGS = -static -mwindows

# arquivos próprios do teste: nunca mexe na caderneta/histórico do bar
TESTE_DEFS = -DARQUIVO_CADERNETA=\"teste_motor_caderneta.txt\" \
             -DARQUIVO_HISTORICO=\"teste_motor_historico.txt\"

all: $(EXE)

$(EXE): $(SRC) $(GUI) $(wildcard src/*.h) gui/gui.h
	$(CC) $(CFLAGS) $(SRC) $(GUI) -Isrc -Igui $(RAYLIB_INC) -o $@ $(RAYLIB_LIB) $(LIBS) $(LDFLAGS)

teste: testes/teste_motor.exe
	./testes/teste_motor.exe

testes/teste_motor.exe: $(SRC) testes/teste_motor.c testes/conta_memoria.h
	$(CC) $(CFLAGS) -Isrc -include testes/conta_memoria.h $(TESTE_DEFS) \
		$(SRC) testes/teste_motor.c -o $@

limpar:
	rm -f $(EXE) testes/*.exe

.PHONY: all teste limpar
