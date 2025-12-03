#! /usr/bin/env bash

mkdir -v -p day_new 
pushd day_new 
mkdir -v src
echo "#include<stdio.h>\nint main() {\n  return;\n}" >> src/main.c
echo "default:\ngcc -g -O3 src/main.c -o out" >> Makefile
popd
