#!/bin/bash -eu

STATEDIR=./stateDir.gwh_run48
mkdir -p $STATEDIR

if [ ! -e $STATEDIR/gxrom-r ]; then
    ( cd $STATEDIR
      wget -c https://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip -O gxrom-r.zip
      unzip gxrom-r.zip
      rm gxrom-r.zip
    )
fi

RAM="-ram ram"
if [ ! -e $STATEDIR/ram ]; then
    RAM="-reset"
fi

if [ ! -e $STATEDIR/port1 ]; then
    dd if=/dev/zero of=$STATEDIR/port1 bs=1k count=128
fi

if [ ! -e $STATEDIR/port2 ]; then
    dd if=/dev/zero of=$STATEDIR/port2 bs=1k count=1024
fi

./run_saturn -face hp48 -hw hp48 -stateDir $STATEDIR -rom gxrom-r "$RAM" -port1 port1 -port2 port2
