#!/bin/bash -eu

make clean
xmkmf
touch saturn.man pack.man
make
