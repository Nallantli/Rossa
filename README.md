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
- [x] Prototypic Functions: `<VALUE>.<FUNCTION>()` wraps to `<FUNCTION>(<VALUE>)`
- [x] Dictionary (key:value list) base-type
- [x] Stack Tracing
- [x] Compilation Error Tracing
- [x] Memory Management
- [x] Unicode Support by Default
- [ ] Multithreading
- [ ] Private Class Members
- [ ] Reflection

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