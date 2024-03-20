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

RAM=''
if [ ! -e $STATEDIR/ram ]; then
    RAM=-reset
fi

[ ! -e $STATEDIR/port1 ] && dd if=/dev/zero of=$STATEDIR/port1 bs=1k count=128
[ ! -e $STATEDIR/port2 ] && dd if=/dev/zero of=$STATEDIR/port2 bs=1k count=1024

./run_saturn -face hp48 -hw hp48 -stateDir $STATEDIR -rom gxrom-r $RAM -port1 port1 -port2 port2
