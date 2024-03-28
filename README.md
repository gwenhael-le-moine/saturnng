# Emulator of the HP 48GX, HP 49, and HP 40

## Building

``` shell
make
```

## Using
``` shell
./dist/run48.sh
```

## Installing
// TODO

## Post-fork changelog
- replaced the build system with a basic Makefile
- updated license from GPL2-or-later to GPL3-or-later
- moved binaries and their dependencies under ./dist/
- new helper scripts run48.sh and run49.sh
- embed ROMs

## Known bugs
- I could get neither the 49 nor the 40 ROM running yet.


**This is a fork of saturn v4.1.1.1 originally by Ivan Cibrario Bertolotti.**

Original source are available at https://www.hpcalc.org/details/4382

Original documentation are kept in ./docs-4.1.1.1/ and ./manual/
