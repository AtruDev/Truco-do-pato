# Bar do Pato — Makefile
# Uso: make        (compila)
#      make run    (compila e executa)
#      make clean  (apaga os arquivos gerados)
# No Windows com MinGW/MSYS2, use "mingw32-make" no lugar de "make".

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Iinclude -Iestruturas -MMD -MP

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)
DEP     = $(OBJ:.o=.d)

ifeq ($(OS),Windows_NT)
    TARGET = bar_do_pato.exe
    RM     = del /Q
    FIXPATH = $(subst /,\,$1)
else
    TARGET = bar_do_pato
    RM     = rm -f
    FIXPATH = $1
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	-$(RM) $(call FIXPATH,$(OBJ) $(DEP) $(TARGET))

-include $(DEP)

.PHONY: all run clean
