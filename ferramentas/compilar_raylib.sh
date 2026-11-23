#!/bin/sh
# Compila a raylib a partir do código-fonte numa libraylib.a estática.
#
# Por quê: em algumas instalações do MSYS2 a libraylib.a pronta (pacman)
# faz o ld fechar sem mensagem ("ld returned 5 exit status"). Compilar
# a raylib com o mesmo gcc resolve.
#
# Uso (no terminal do MSYS2 UCRT64 ou no Git Bash com o gcc no PATH):
#   1. baixe e descompacte https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
#   2. sh ferramentas/compilar_raylib.sh caminho/para/raylib-6.0 raylib
#   3. mingw32-make RAYLIB_DIR=raylib
set -e
FONTE="$1/src"
SAIDA="${2:-raylib}"
[ -f "$FONTE/raylib.h" ] || { echo "Nao achei $FONTE/raylib.h"; exit 1; }
mkdir -p "$SAIDA"
for f in rcore rshapes rtextures rtext rmodels raudio rglfw; do
    echo "compilando $f.c"
    gcc -c -O2 -std=gnu99 -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33 \
        -I"$FONTE" -I"$FONTE/external/glfw/include" "$FONTE/$f.c" -o "$SAIDA/$f.o"
done
rm -f "$SAIDA/libraylib.a"
ar rcs "$SAIDA/libraylib.a" "$SAIDA"/*.o
rm -f "$SAIDA"/*.o
cp "$FONTE/raylib.h" "$FONTE/raymath.h" "$FONTE/rlgl.h" "$SAIDA/"
echo "pronto: $SAIDA/libraylib.a"
