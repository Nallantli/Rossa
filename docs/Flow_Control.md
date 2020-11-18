# Flow Control

With regards to syntax of control structures, the basis rests mainly on the notion of familiarity and simplicity. For this reason, the control syntax is not parallel with any other major programming language, but the components piecing it together are unilaterally familiar.

There are common element in all control structures: the general syntax of a control-block's initial circumstance is bound by two keywords, e.g. `if ... then` where the circumstantial expression is placed within. Control structures are then given a body within scoping brackets `{}`.

Additionally, expressions declared between the control keywords are evaluated as being scoped within the control-block's body:

```ruo
if a := true then { # `a` is declared with the value `true`
	# `a` can be accessed here
} # `a` is deleted as the scope terminates

# `a` a cannot be accessed here
```

Further illustrations of this dynamic will be demonstrated with the while-loop.

## If Statement

If statements are perhaps the most important control structure in a programming language, and for this best we not bereave ourselves of a simple way to manage the function.

The general syntax is as follows:

```ruo
if <BOOLEAN> then {

}
```

Note that the element designated by `<BOOLEAN>` must reflect an expression that evaluates to a `Boolean` value. An if-statement will and will only accept such an expression. Otherwise the interpreter will throw an error during evaluation.

To demonstrate, a quick example:

```ruo
var a;

# the variable `a` is given a value somewhere

if a == 1 then {
	putln("Hello, world!");
}
```

In this example, the expression within the if-statement will only evaluate where `a` is of type `Number`, and of value `1`.

Note the lack of parentheses surrounding the statement, a general mark of C-based languages. Putting parentheses around the statement is most certainly a possibility, but it is superfluous as the boundaries are designated by the keywords `if` and `then`.

If-statements can be given an `else` clause likewise:

```ruo
if <BOOLEAN> then {
	...
} else {
	...
}
```

This `else` encompasses all situations in which the `<BOOLEAN>` expression evaluates to `false`. To specify more cases, `elif` is used:

```ruo
if a == 1 then {
	...
} elif a == 2 then {
	...
} elif a == 3 then {
	...
} else {
	...
}
```

An if-statement's `else` clause must be the final clause. There is no limit to the amount of `elif` clauses that may be appended.

Underlyingly, the former example is equivalent to:

```ruo
if a == 1 then {
	...
} else {
	if a == 2 then {
		...
	} else {
		if a == 3 then {
			...
		} else {
			...
		}
	}
}
```

## While Loop

The while-loop is a structure that continues executing its body until that initial statement is evaluated to be `false`. In short, before the execution of the structure's body, the condition is reëvaluated in its original form.

The keywords binding the while-loop are `while` and `do`, used in the same manner as the if-statement above:

```ruo
while true do {
	...
}
```

Note that this loop will continue indefinitely, as the condition always evaluates to `true`, because it is `true`.

For an example using a proper conditional:

```ruo
while (line := input.line()) != "quit" do {
	putln(line);
}
```

This short sequence requests user input at each loop, and checks if it is equal to the `String` value `"quit"`. If this is the case, it does not continue, otherwise, it prints the user's text unto the console.

There is an important component here that was discussed in the parent category: the `while` structure's statement declares the variable `line` inside of the clause, which generates it as available for the interior scope of the while-loop. While the following is possible:

```ruo
line := input.line();
while line != "quit" do {
	line = input.line();
	putln(line);
}
```

The former is computationally simpler and efficient by comparison. Additionally, in the latter the variable `line` can be accessed elsewhere in the upper scope rather than being a sole responsibility of the while-loop.

## For Loop

The for-loop structure traditionally is composed of 3 statements, a declaration, a comparison, and an increment:

```c
for (int i = 0; i < 10; i++) {
	...
}
```

This structure is, in this developer's view, unnecessarily explicative for high-level languages. The above C code is functionally equivalent to the following:

```c
int i = 0;
while (i < 10) {
	...
	i++;
}
```

For this reason it is not really within the goal of this language to have a structure immediately corresponding to a declare-compare-augmenter.

What is becoming more widely available to programmers is the for-each structure (in C++/Java this uses `for (<ELEMENT> : <ITERABLE>)`, JS `Array.protoype.forEach()`, etc.). This procedure makes more sense for a high level of abstraction compared to the DCA format, and in this vein Ruota reflects for-each only:

```ruo
for i in 0 .. 10 do {
	...
}
```

The for-loop is composed of 3 statements, the first being a variable identifier (here `i`), followed by the keyword `in`, and then a designated `Array` value to iterate over.

In the above example, the operataor `..` generates a set of values from `0` to `10` exclusive (i.e. `[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]`). `Array` values outside of for-loops can be declared with the same syntax:

```ruo
a := [0 .. 10];
```

However within a for-loop, the brackets are not required.

Additional range operations include `.+` which produces an _inclusive_ set (`[0 .+ 10]` being `[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]`), and a ternary component to specify step with `:`.

In this way, constructing a list of values from `0` to `100`, inclusive, where the values are all even:

```ruo
a := [0 .+ 100 : 2];
```

In the for-loop, and `Array` value that is iterated upon is mutable, and the values of the `Array` need not be restricted to `Number` types:

```ruo
a := [
	"hello",
	"ciao",
	"salve",
	"こんにちは"
];

for e in a do {
	e += ", world!";
	putln(e);
}
```

Output here being:

```
hello, world!
ciao, world!
salve, world!
こんにちは, world!
```

These are also the new values of the `Array` elements, as they are set new values within the for-loop.

## Switch Statement

The switch-statement is an important control sequence for when an if-statement might just have [too many else-if clauses](https://www.youtube.com/watch?v=r97nq2h8k9M).

A switch-statement allows the code to branch based on a hashed table of possible values. In C/++ these values are limited to constant integral values. Here, all base data types are acceptable candidates for a switch case.

The syntax is as follows:

```ruo
switch <EXPRESSION> in {
	case <VALUE> do {
		...
	}
	...
}
```

For the sake of brevity, a case-statement may be given in a single line:

```ruo
case <VALUE> do <EXPRESSION>;
```

The `default` case for a switch statement is defined with an `else` clause coming after the switch block, like so:

```ruo
switch <EXPRESSION> in {
	...
} else {
	...
}
```

The key difference in Ruota switch statements compared to languages like C/++ and Java is that there is no need for `break` to stop flow into the proceeding case. All cases are self-contained paths.

The switch-statement can also have cases for values that are not constant or base types, these values will be evaluated at the time the switch statement is called. In this manner the switch statement will operate in a manner kin to an if-else sequence. Priority will be given to _constant_ values that are evaluated during compilation and placed in the switch map.

For instance:

```ruo
switch a in {
	case 1 do {
		...
	}
	case 2 do {
		...
	}
	case f() do {
		...
	}
}
```

Here the switch statement will first compare `a` to the constant cases (`1` and `2`) and if neither match, will proceed to evaluate the runtime case `f()`. Even if `f()` evaluates to `1` or `2`, the above cases will have already been selected first.