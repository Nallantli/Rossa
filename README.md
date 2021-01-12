<div align="center">
<h1>Rossa</h1>
<img src="docs/Logo.svg" width="200">
</div>

-----
<div align="center">

[<img src="misc/code.png" width="400">](test/fibonacci.ra)

![Rossa](https://github.com/Nallantli/Rossa/workflows/Rossa/badge.svg) ![libfs](https://github.com/Nallantli/Rossa/workflows/libfs/badge.svg) ![libnet](https://github.com/Nallantli/Rossa/workflows/libnet/badge.svg) ![libsdl](https://github.com/Nallantli/Rossa/workflows/libsdl/badge.svg)

</div>

[Docs](https://nallantli.github.io/Rossa/#/)

-----

## Building

### Libraries

There will be no downloadable binaries during the alpha stage - however everything can be built from the source rather easily.

Building the main interpreter requires no exteral libraries.

Individual Libraries require certain libraries:

*libsdl*

* SDL (https://www.libsdl.org/)
* SDL_image (https://www.libsdl.org/projects/SDL_image/)
* SDL_ttf (https://www.libsdl.org/projects/SDL_ttf/)

*libnet*

* Boost (https://www.boost.org/)

On Linux/OSX these libraries can be installed using a package manager. E.g. Debian-based systems:

```sh
sudo apt-get install libboost-all-dev
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
sudo apt-get install libsdl2-ttf-dev
```

On Windows, you will have to download the development libraries from the websites above and then edit the paths in the makefile.

### Cloning

```sh
git clone https://github.com/Nallantli/Rossa
cd Rossa
make
```

`make` can also be used with optional `locale` to specify a language. Current options are `ENG` (Default), `LAT`, `JPN`, `ITA`.

e.g. `make locale=LAT`

Changing the locale variable merely alters the language of the error messages.

The binary will be located in `bin/` and the dll/so libraries in `bin/lib/`.

-----

## Basics

Hello World:

```ra
putln("Hello World!");
```

Factorial:

```ra
fact(x) => {
	if x > 1 then {
		return x * fact(x - 1);
	} else {
		return 1;
	}
}

putln(fact(5)); # 120
```

FizzBuzz:

```ra
for i in 1 .+ 100 do {
	if i % 3 == 0 && i % 5 == 0 then {
		putln("FizzBuzz");
	} elif i % 3 == 0 then {
		putln("Fizz");
	} elif i % 5 == 0 then {
		putln("Buzz");
	} else {
		putln(i);
	}
}
```

Basic File/Folder Output:
(Note that the `/` operator is overloaded in class `Path`)

```ra
load "fs";

p := new Path();
p = p / "Example";
if !p.exists() then {
	p.mkdirs();
}

io := new FileIO(p / "example.txt");
io.write("Hello World");
io.close();
```

Threading:

```ra
f(ref x : Number, ref id : Number) => {
	for i in 0 .. x do {
		putln("Thread " + id -> String + " says " + i -> String);
	}
}

t1 := new Thread(|> f(10, 1));
t2 := new Thread(|> f(10, 2));

t1.join();
t2.join();
```