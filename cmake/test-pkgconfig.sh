#!/bin/sh
# Test if including/linking/running an installed raylib works

set -x
export LD_RUN_PATH=/usr/local/lib

CFLAGS="-Wall -Wextra -Werror $CFLAGS"
if [ "$ARCH" = "i386" ]; then
CFLAGS="-m32 $CLFAGS"
fi

cat << EOF | ${CC:-cc} -otest -xc - $(pkg-config --libs --cflags $@ raylib.pc) $CFLAGS && exec ./test
#include <stdlib.h>
#include <raylib.h>

int main(void)
{
    int num = GetRandomValue(42, 1337);
    return 42 <= num && num <= 1337 ? EXIT_SUCCESS : EXIT_FAILURE;
}
EOF
