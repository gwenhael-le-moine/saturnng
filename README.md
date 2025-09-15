# Emulator of the HP 49G, HP 48 (GX & SX), and HP 40G

**This is a fork of saturn v4.1.1.1 originally by Ivan Cibrario Bertolotti.**

Original source are available at https://www.hpcalc.org/details/4382

**saturnng** is part of my little collection of HP calculators' emulators' that I took over maintenance :

- [x50ng](https://codeberg.org/gwh/x50ng)
- [x48ng](https://codeberg.org/gwh/x48ng)
- [saturnng](https://codeberg.org/gwh/saturnng) ← You are here
- [hpemung](https://codeberg.org/gwh/hpemung)

[ The reference public source repository is https://codeberg.org/gwh/saturnng ]

The GUI has been replaced by a new one (taken from x48ng) in SDL2 and/or ncurses.

## Screenshots

![screenshot of saturn49g](./saturn49g.png?raw=true "screenshot of saturn49g")
![screenshot of saturn48gx](./saturn48gx.png?raw=true "screenshot of saturn48gx")
![screenshot of saturn48sx](./saturn48sx.png?raw=true "screenshot of saturn48sx")

## Building

Dependencies:
- SDL3
- ncursesw


``` shell
make
```

## Installing
``` shell
make install DESTDIR=/
```

## Using
The main binary is `dist/saturn` with helpers/wrappers scripts available per model as:
* `dist/saturn48gx`
* `dist/saturn48sx`
* `dist/saturn49g`
* `dist/saturn40g` (not really functional (yet))

The local data are stored under `$XDG_CONFIG_HOME/saturn<model>/`.

The scripts will take care of creating `$XDG_CONFIG_HOME/saturn<model>/`, download an appropriate ROM from hpcalc.org and create RAM cards (for 48gx and 48sx models.)

## Known bugs
- some bugs in emulation:
  - 48gx: ON-D A can hang because it tries to write data in ROM space
  - 48gx: VERSION spouts messages in the console
  - 49g: spouts messages in the console every second

## Todo
- fix emulation bugs
- 49g: find a way to enable the bigger screen (131×80)
- 40g: make emulation work
