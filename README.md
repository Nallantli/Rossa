<div align="center">
<h1>Rossa</h1>
<img src="docs/Logo.svg" width="200">
</div>

-----
<div align="center">

![C/C++ CI](https://github.com/Nallantli/Rossa/workflows/C/C++%20CI/badge.svg)

</div>

[Docs](https://nallantli.github.io/Rossa/#/)

[Formerly named _Ruota_](misc/namechange.md)

-----

## Building

### Libraries

There will be no downloadable binaries during the alpha stage - however everything can be built from the source rather easily.

Building requires the following libraries:

* Boost (https://www.boost.org/)

For the SDL integration:

* SDL (https://www.libsdl.org/)
* SDL_image (https://www.libsdl.org/projects/SDL_image/)

On Linux/OSX these libraries can be installed using a package manager. E.g. Debian-based systems:

```sh
sudo apt-get install libboost-all-dev
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev
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

```rossa
putln("Hello World!");
```

Factorial:

```rossa
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

```rossa
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

```rossa
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

```rossa
f(x, id) => {
	for i in 0 .. x do {
		putln("Thread " + (id -> String) + ": " + (i -> String));
	}
}

t1 := new Thread(() => f(10, 1));
t2 := new Thread(() => f(10, 2));

t1.join();
t2.join();
```

-----

## Operators

Token|Overridable?|Compound Assignment?*|Usage| Associativity|Precedence
-|-|-|-|-|-
`a.b`|No|No|Object Indexing|Right|15
`a -> b`|No|No|Value Casting `a` to `b`, e.g. `"1" -> Number == 1`|Left|14
`a ** b`|Yes|Yes|Exponent|Right|13
`a * b`|Yes|Yes|Multiplication|Left|12
`a / b`|Yes|Yes|Division|Left|12
`a + b`|Yes|Yes|Addition, Concatanation|Left|11
`a - b`|Yes|Yes|Subtraction|Left|11
`a << b`|Yes|Yes|Left Bit-shift|Left|10
`a >> b`|Yes|Yes|Right Bit-shift|Left|10
`a < b`|Yes|No|`a` Less Than `b`|Left|8
`a > b`|Yes|No|`a` Greater Than `b`|Left|8
`a <= b`|Yes|No|`a` Less Than `b` or Equal|Left|8
`a >= b`|Yes|No|`a` Greater Than `b` or Equal|Left|8
`a == b`|Yes|No|`a` Equals `b`|Left|7
`a != b`|Yes|No|`a` Does Not Equal `b`|Left|7
`a === b`|No|No|Equivalent to `==` in all cases except where `==` is overridden for an object value - this compares equivalency based on reference.|Left|7
`a !== b`|No|No|See Above|Left|7
`a & b`|Yes|Yes|Bit-Wise And|Left|6
`a ^ b`|Yes|Yes|Bit-Wise XOr|Left|5
`a \| b`|Yes|Yes|Bit-Wise Or|Left|4
`a && b`|Yes|Yes|Logical And|Left|3
`a \|\| b`|Yes|Yes|Logical Or|Left|2
`a .. b`|Yes|~|Range of values from `a` to `b`|Left|1
`a = b`|Yes|~|Value Assignment|Right|0
`a := b`|No|~|Variable Declaration and Assignment|Right|0
`a[b]`|Yes (via `` `[]` ``)|~|Indexing|~|~
`a(b,...)`|Yes (via `` `()` ``)|~|Function Calling|~|~
`!a`|No**|~|Unary Logical Negation: `a == false`|~|~
`+a`|No**|~|Unary Addition (Superfluous in most cases): `0 + a`|~|~
`-a`|No**|~|Unary Negation: `0 - a`|~|~

_\* Can be used in compound-assignment operators, i.e. `a += b`. This is functionally equivalent to `a = a + b`._

_\*\* The interpreter expands these operators to their binary equivalents during pre-compilation, consequently if their binary equivalents have been overridden, they too will reflect that._