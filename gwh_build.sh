#!/bin/bash -eu

xmkmf

make clean
rm src/*.o

make depend
make
