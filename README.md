# Emulator of the HP 48GX, HP 49, and HP 40

**This is a fork of saturn v4.1.1.1 originally by Ivan Cibrario Bertolotti.**

Original source are available at https://www.hpcalc.org/details/4382

The GUI has been replaced by a new one (taken from x48ng) in SDL2 and/or ncurses.

The main binary is `dist/saturn` with helpers/wrappers scripts available per model as:
* `dist/saturn48gx`
* `dist/saturn48sx`
* `dist/saturn49g`
* `dist/saturn40g` (not really functional (yet))

## Screenshots

![screenshot of saturn49g](./saturn49g.png?raw=true "screenshot of saturn49g")
![screenshot of saturn48gx](./saturn48gx.png?raw=true "screenshot of saturn48gx")
![screenshot of saturn48sx](./saturn48sx.png?raw=true "screenshot of saturn48sx")

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

## Known bugs
- some bugs in emulation:
  - 48gx: ON-D A can hang because it tries to write data in ROM space
  - 48gx: VERSION spouts messages in the console (but works)
  - 49g: spouts messages in the console every second (but works)
- ncurses UI: becomes unresponsive (but still quits gracefully on F7)

## Todo
- fix emulation bugs
- 49g: find a way to enable the bigger screen (131×80)
- 40g: make emulation work
