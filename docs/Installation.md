# Quick Start

## Cloning

Clone from the GitHub repo:

```bash
git clone https://github.com/Nallantli/Rossa
cd Rossa
```

You will also need the following librar(y/ies) in order to build:

* Boost (https://www.boost.org/)

For `libsdl`, these additionally will be necessary:

* SDL (https://www.libsdl.org/)
* SDL_image (https://www.libsdl.org/projects/SDL_image/)

The first is used ubiquituously by the interpreter, and the second two SDL dependencies related solely to `libsdl.cpp` and its corresponding `.ruo` file `bin/lib/sdl.ruo`.

## *Nix

On *nix systems, installing these libraries is very simple. Most package managers have all three dependencies available. The following example is for Debian-based distros:

```bash
sudo apt-get install libboost-all-dev
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
```

## Windows

As for Windows, the binaries will need to be downloaded from the websites linked above. After which, it will be necessary to change paths in the Makefile to link the libraries properly.

```makefile
# Path to the directory containing the boost development library
BOOST_PATH_WIN=C:/boost
# Path to the directory containing the SDL2 development library
SDL_PATH_WIN=C:/SDL2/x86_64-w64-mingw32
# Version of boost
BOOST_VERSION_WIN=1_73
# Most likely will not need to be changed unless you are using the x86 version
SUFFIX_WIN=-mgw8-mt-x64-$(BOOST_VERSION_WIN)
# Path to the directory containing the SDL2 Image development library
SDL_IMAGE_PATH_WIN=C:/SDL2_image/x86_64-w64-mingw32
```

## Building

All systems need only type the `make` command.

```bash
make
```

The libraries (other than the standard `libstd`/`sdl.ruo`) need to be compiled before use:

```bash
make libnet
make libfs
make libsdl
```

Alternatively, `make libs` compiles all of the above.