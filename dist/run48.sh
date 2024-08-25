#!/bin/bash -eu

cd "$(dirname "$0")" ; CWD=$(pwd)

STATEDIR=$CWD/stateDir.48
mkdir -p "$STATEDIR"

if [ ! -e "$STATEDIR"/gxrom-r ]; then
    make -C .. get-roms
    cp ROMs/gxrom-r "$STATEDIR"/gxrom-r
fi

RAM=''
if [ ! -e "$STATEDIR"/ram ]; then
    RAM=-reset
fi

[ ! -e "$STATEDIR"/port1 ] && dd if=/dev/zero of="$STATEDIR"/port1 bs=1k count=128
[ ! -e "$STATEDIR"/port2 ] && dd if=/dev/zero of="$STATEDIR"/port2 bs=1k count=4096

./run_saturn -face hp48 -hw hp48 -stateDir "$STATEDIR" -rom gxrom-r $RAM -port1 port1 -port2 port2
