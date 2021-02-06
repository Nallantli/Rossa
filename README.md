<div align="center">
<h1>Rossa</h1>
<img src="docs/Logo.svg" width="200">
</div>

-----

<div align="center">

![Rossa](https://github.com/Nallantli/Rossa/workflows/Rossa/badge.svg) ![libstd](https://github.com/Nallantli/Rossa/workflows/libstd/badge.svg) ![libfs](https://github.com/Nallantli/Rossa/workflows/libfs/badge.svg) ![libnet](https://github.com/Nallantli/Rossa/workflows/libnet/badge.svg) ![libsdl](https://github.com/Nallantli/Rossa/workflows/libsdl/badge.svg) ![libncurses](https://github.com/Nallantli/Rossa/workflows/libncurses/badge.svg) [![License](https://img.shields.io/badge/license-BSD%203--Clause-blue)](LICENSE)

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

Fibonacci Sequence:

```ra
fib(ref size : Number) => {
	array := [0, 1] ++ alloc(size - 2);
	for i in 2 .. size do {
		array[i] = array[i - 1] + array[i - 2];
	}
	return array;
}

fib(20).map((e, i) => putln("{0}:\t{1}" & [i, e]));
```

FizzBuzz:

```ra
for i in 1 <> 100 do {
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
		putln("Thread {0} says {1}" & [id, i]);
	}
}

t1 := new Thread(()[f] => f(10, 1));
t2 := new Thread(()[f] => f(10, 2));

t1.join();
t2.join();
```