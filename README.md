<div align="center">
<h1>Ruota</h1>
<img src="docs/logo/Logo.svg" width="200">
</div>

-----

_Current features, divergences from previous repo, and features to be implemented:_

- [x] External Library Integration at Runtime (C++)
- [x] Classes with Inheritence
- [x] Operator Overloading
- [x] Static Typing
- [x] Static Casting
- [x] Function Overloading (by argument size)
- [x] Prototypic Functions: `<VALUE>.<FUNCTION>(...)` wraps to `<FUNCTION>(<VALUE>,...)`
- [x] Dictionary (key:value list) base-type
- [x] Stack Tracing
- [x] Compilation Error Tracing
- [x] Memory Management
- [x] Unicode Support by Default
- [ ] Multithreading
- [ ] Private Class Members
- [ ] Reflection

-----

## Building

There will be no downloadable binaries during the alpha stage - however everything can be built from the source rather easily.

```sh
git clone https://github.com/Nallantli/Ruota
cd Ruota
make dirs
make
```

`make` can also be used with optional `locale` to specify a language. Current options are `ENG` (Default), `LAT`, `JPN`.

e.g. `make locale=LAT`

Changing the locale variable merely alters the language of the error messages.

The binary will be located in `bin/` and the dll/so libraries in `bin/lib/`.

-----

## Basics

Hello World:

```ruota
putln("Hello World!");
```

Factorial:

```ruota
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

```ruota
for i in [1 until 100] do {
	if i % 3 == 0 && i % 5 == 0 then {
		putln("FizzBuzz");
	} elseif i % 3 == 0 then {
		putln("Fizz");
	} elseif i % 5 == 0 then {
		putln("Buzz");
	} else {
		putln(i);
	}
}
```

Basic File/Folder Output:
(Note that the `/` operator is overloaded in class `Path`)

```ruota
load "fs.ruo";

var p = new Path();
p = p / "Example";
if !p.exists() then {
	p.mkdirs();
}

var io = new FileIO(p / "example.txt");
io.write("Hello World");
io.close();
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
`a | b`|Yes|Yes|Bit-Wise Or|Left|4
`a && b`|Yes|No|Logical And|Left|3
`a || b`|Yes|No|Logical Or|Left|2
`a = b`|Yes|~|Value Assignment|Right|1
`a[b]`|Yes|~|Indexing|~|~
`a(b,...)`|Yes|~|Function Calling|~|~
`!a`|No**|~|Unary Logical Negation|~|~
`+a`|No|~|Unary Addition (Superfluous in most cases)|~|~
`-a`|No|~|Unary Negation|~|~

_\* Can be used in compound-assignment operators, i.e. `a += b`. This is functionally equivalent to `a = a + b`._

_\*\* The interpreter expands these operators to their binary equivalents during pre-compilation, consequently if their binary equivalents have been overridden, they too will reflect that. E.g. `!a` expands to `a == false`._