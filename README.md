<div align="center">
<h1>Rossa</h1>
<img src="docs/Logo.svg" width="200">
</div>

-----

<div align="center">

![Rossa](https://github.com/Nallantli/Rossa/workflows/Rossa/badge.svg) 

</div>

<div align="center">

![standard](https://github.com/Nallantli/Rossa/workflows/standard/badge.svg) ![fs](https://github.com/Nallantli/Rossa/workflows/fs/badge.svg) ![net](https://github.com/Nallantli/Rossa/workflows/net/badge.svg) ![graphics](https://github.com/Nallantli/Rossa/workflows/graphics/badge.svg) ![SDL](https://github.com/Nallantli/Rossa/workflows/SDL/badge.svg) ![ncurses](https://github.com/Nallantli/Rossa/workflows/ncurses/badge.svg) ![Arbitrary](https://github.com/Nallantli/Rossa/workflows/Arbitrary/badge.svg) ![Thread](https://github.com/Nallantli/Rossa/workflows/Thread/badge.svg)

</div>

<div align="center">

[![License](https://img.shields.io/badge/license-BSD%203--Clause-blue)](LICENSE)

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
fn fact(x) {
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
fn fib(ref size: Number) {
	array := [0, 1] ++ alloc(size - 2);
	for i in 2 .. size do {
		array[i] = array[i - 1] + array[i - 2];
	}
	return array;
}

fib(20).map(fn(e, i) putln("{0}:\t{1}" & [i, e]));
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
load "Thread"

fn f(ref x: Number, ref id: Number) {
	for i in 0 .. x do {
		putln("Thread {0} says {1}" & [id, i]);
	}
}

t1 := new Thread(fn()[f] f(10, 1));
t2 := new Thread(fn()[f] f(10, 2));

t1.join();
t2.join();
```

---

_Making languages_

_is very fun, but I wish_

_that it worked well too_
