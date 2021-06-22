# Quick Start

## Cloning

Clone from the GitHub repo:

```bash
git clone https://github.com/Nallantli/Rossa
cd Rossa
```

You will also need the following librar(y/ies) in order to build certain libraries:

*libsdl*

* SDL
  * https://www.libsdl.org/
  * [_zlib License_](https://www.libsdl.org/license.php)
* SDL_image
  * https://www.libsdl.org/projects/SDL_image/
  * _zlib License_
* SDL_ttf
  * https://www.libsdl.org/projects/SDL_ttf/
  * _zlib License_

*libnet*

* Boost
  * https://www.boost.org/
  * [_Boost License_](https://www.boost.org/users/license.html)

*libfs*

* libzip
  * https://libzip.org/
  * [_BSD 3-Clause License_](https://libzip.org/license/)

## *Nix

On *nix systems, installing these libraries is very simple. Most package managers have all three dependencies available. The following example is for Debian-based distros:

```bash
sudo apt-get install libboost-all-dev
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev
sudo apt-get install libzip-dev
```

## Windows

There are various ways to attain the packages - my recommendation is through [MSYS2](https://www.msys2.org/) which can use `pacman` to fetch the entire MinGW toolchain, and all the libraries above.

No matter how you install them, make sure that they are in your compiler's path.

## Building

### Interpreter

All systems need only type the `make` command.

```bash
make
```

`make` can also be used with optional `locale` to specify a language. Current options are `ENG` (Default), `LAT`, `JPN`, `ITA`.

e.g. `make locale=LAT`

Changing the locale variable merely alters the language of the error messages.

The binary will be located in `bin/` and the dll/so libraries in `bin/lib/`.

### Libraries

The libraries (other than the standard `libstd`/`sdl.ra`) need to be compiled before use:

```bash
make libnet
make libfs
make libsdl
```

Alternatively, `make libs` compiles all of the above.