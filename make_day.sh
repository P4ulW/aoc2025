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
echo $'#include<stdio.h>\nint main() {\n\treturn;\n}' >> src/main.c
echo $'default:\n\tgcc -g -O3 src/main.c -o out' >> Makefile
popd
echo "$dirname successfully created"
exit 0
