# Emulator of the HP 48GX, HP 49, and HP 40

**This is a fork of saturn v4.1.1.1 originally by Ivan Cibrario Bertolotti.**

Original source are available at https://www.hpcalc.org/details/4382

The GUI has been replaced by a new one (taken from x48ng) in SDL2 and/or ncurses.

The main binary is `dist/saturn` with helpers/wrappers scripts available per model as:
* `dist/saturn48gx`
* `dist/saturn48sx`
* `dist/saturn49g` (not really functional (yet))
* `dist/saturn40g` (not really functional (yet))

## Building

Dependencies:
- SDL2
- ncursesw


``` shell
make
```

## Installing
``` shell
make install DESTDIR=/
```

## Using
Use the wrappers scripts.

The local data are stored under $XDG_CONFIG_HOME/saturn<model>/

## Post-fork changelog
- replaced the build system with a basic Makefile
- updated license from GPL2-or-later to GPL3-or-later
- moved binaries and their dependencies under ./dist/
- new helper scripts run48.sh and run49.sh
- ROMs helper Makefile

## Known bugs
- I could get neither the 49 nor the 40 ROM running yet.
