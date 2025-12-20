#! /usr/bin/env bash

dirname="day"$1
if [ -d "$dirname" ]; then
    echo "WARNING: $dirname alreay exists, not creating new one!"
    exit -1
fi

mkdir -v -p $dirname 
pushd $dirname
mkdir -v src
pushd src
git clone git@github.com:P4ulW/cbase.git
pushd cbase
git checkout dev
rm .git -rvf
popd
popd
echo $'#include <stddef.h>\n#include <stdio.h>\n#include "cbase/src/ansi_codes.h"\n#include "cbase/src/arena.c"\n#include "cbase/src/array.c"\n#include "cbase/src/string.c"\n\n#define EXAMPLE 1\n\n#if EXAMPLE == 0\n#define FILENAME "test.txt"\n#else\n#define FILENAME "input.txt"\n#endif\n\nint main() {\n\treturn 0;\n}' >> src/main.c
echo $'default:\n\tgcc -g -O3 src/main.c -o out' >> Makefile
popd
echo "$dirname successfully created"
exit 0
