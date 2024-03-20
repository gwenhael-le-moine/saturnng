#!/bin/bash -eu

STATEDIR=./stateDir.gwh_run48
mkdir -p $STATEDIR

if [ ! -e $STATEDIR/gxrom-r ]; then
    (cd $STATEDIR
     wget -c https://www.hpcalc.org/hp48/pc/emulators/gxrom-r.zip -O gxrom-r.zip
     unzip gxrom-r.zip
     rm gxrom-r.zip
    )
fi
RAM=ram
if [ ! -e $STATEDIR/ram ]; then
    RAM="-reset"
fi

./run_saturn -face hp48 -hw hp48 -stateDir $STATEDIR -rom gxrom-r $RAM
