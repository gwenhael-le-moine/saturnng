#!/bin/bash -eu

make clean
xmkmf

make depend
make
