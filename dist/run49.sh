#!/bin/bash -eu

cd "$(dirname "$0")" ; CWD=$(pwd)

STATEDIR=$CWD/stateDir.49
mkdir -p "$STATEDIR"

if [ ! -e "$STATEDIR"/rom.49g ]; then
    ( cd "$STATEDIR"
      # wget -c https://www.hpcalc.org/hp49/pc/rom/hp4950v210.zip -O rom.zip
      wget -c https://www.hpcalc.org/hp49/pc/rom/beta1196.zip -O rom.zip
      unzip rom.zip
      rm rom.zip
    )
fi

RAM=''
if [ ! -e "$STATEDIR"/ram ]; then
    RAM=-reset
fi

./run_saturn -face hp49 -hw hp49 -stateDir "$STATEDIR" -rom rom.49g $RAM
