<div align="center">
<h1>Rossa</h1>
<img src="docs/Logo.svg" width="200">
</div>

-----

<div align="center">

[<img src="misc/code.png" width="400">](test/tpk.ra)

![Rossa](https://github.com/Nallantli/Rossa/workflows/Rossa/badge.svg) ![libfs](https://github.com/Nallantli/Rossa/workflows/libfs/badge.svg) ![libnet](https://github.com/Nallantli/Rossa/workflows/libnet/badge.svg) ![libsdl](https://github.com/Nallantli/Rossa/workflows/libsdl/badge.svg) [![License](https://img.shields.io/badge/license-BSD%203--Clause-blue)](LICENSE)

</div>

-----

## Building & Installation

See [Installation](https://nallantli.github.io/Rossa/#/Installation)

## Documentation

Scant, but progressing: https://nallantli.github.io/Rossa/#/

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